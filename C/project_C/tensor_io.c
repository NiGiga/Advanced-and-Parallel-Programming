/* Nicola Gigante - SM3201239
 * tensor_io.c
 * I/O binario per i tensori nel formato on_disk_tensor.
 * In lettura uso mmap e faccio puntare tensor->data direttamente ai dati
 * mappati dal file, senza copiare in un buffer separato.
 */

#include "tensor_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>      /* open */
#include <sys/mman.h>   /* mmap, munmap */
#include <sys/stat.h>   /* fstat */
#include <unistd.h>     /* close */

/* Carica un tensore da file usando mmap, senza copiare i dati.
 * Formato del file (come da specifica):
 *
 *   struct on_disk_tensor {
 *       int32_t shape[MAX_DIM];
 *       int32_t ndim;
 *       off_t   data_offset;
 *   };
 *
 *   // padding fino a data_offset (64 bytes per il progetto)
 *   float data[...]  // dati del tensore memorizzati contiguamente
 *
 * La funzione:
 *   - mappa l'intero file
 *   - interpreta l'header on_disk_tensor
 *   - controlla che ci siano abbastanza dati
 *   - crea una struct tensor che punta direttamente ai dati mappati
 *   - salva mmap_base/mmap_size in modo che tensor_dec_ref possa fare munmap.
 */
tensor *tensor_load_from_file_mmap(const char *filename) {
    /* Apro il file in sola lettura. */
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        perror("tensor_load_from_file_mmap: open");
        return NULL;
    }

    /* Ottengo dimensione e metadati del file. */
    struct stat st;
    if (fstat(fd, &st) != 0) {
        perror("tensor_load_from_file_mmap: fstat");
        close(fd);
        return NULL;
    }

    /* Verifico che il file sia abbastanza grande per contenere almeno l'header. */
    if (st.st_size < (off_t)sizeof(on_disk_tensor)) {
        close(fd);
        return NULL;
    }

    /* Mappo tutto il file in memoria in sola lettura. */
    void *map = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);  /* dopo mmap posso chiudere il file descriptor. */
    if (map == MAP_FAILED) {
        perror("tensor_load_from_file_mmap: mmap");
        return NULL;
    }

    /* Interpreto l'inizio del file come un header on_disk_tensor. */
    on_disk_tensor *header = (on_disk_tensor *)map;

    /* Controllo che ndim sia valido. */
    if (header->ndim <= 0 || header->ndim > MAX_DIM) {
        munmap(map, st.st_size);
        return NULL;
    }

    /* Copio le dimensioni in un array temporaneo e le verifico. */
    int32_t shape[MAX_DIM];
    for (int i = 0; i < header->ndim; ++i) {
        if (header->shape[i] <= 0) {
            /* Dimensione non positiva => formato non valido. */
            munmap(map, st.st_size);
            return NULL;
        }
        shape[i] = header->shape[i];
    }

    /* Calcolo il numero totale di elementi atteso. */
    size_t nelems = tensor_num_elements(shape, header->ndim);
    if (nelems == 0) {
        munmap(map, st.st_size);
        return NULL;
    }

    /* Verifico che data_offset + spazio dati rientrino nella dimensione del file. */
    off_t data_start = header->data_offset;
    off_t data_end   = data_start + (off_t)(nelems * sizeof(float));
    if (data_start < 0 || data_end > st.st_size) {
        /* I dati non stanno interamente nel file. */
        munmap(map, st.st_size);
        return NULL;
    }

    /* Calcolo il puntatore ai dati float all'interno della mappatura. */
    float *data_on_disk = (float *)((char *)map + header->data_offset);

    /* Alloco solo la struct tensor, SENZA allocare un nuovo buffer dati. */
    tensor *t = malloc(sizeof(tensor));
    if (!t) {
        munmap(map, st.st_size);
        return NULL;
    }

    /* Inizializzo i campi del tensore in modo consistente. */
    t->data = data_on_disk;
    t->ndim = header->ndim;
    for (int i = 0; i < MAX_DIM; ++i) {
        t->shape[i] = (i < header->ndim) ? header->shape[i] : 0;
    }
    t->size = nelems;
    t->refcount = 1;

    /* Salvo le informazioni sulla mappatura: 
     * - mmap_base: indirizzo base dell'area mappata
     * - mmap_size: dimensione totale della mappatura (in bytes)
     * Saranno usate da tensor_dec_ref per fare munmap quando il tensore non serve più.
     */
    t->mmap_base = map;
    t->mmap_size = (size_t)st.st_size;

    return t;
}

/* Salva un tensore su disco nel formato on_disk_tensor.
 * Layout del file:
 *
 *   on_disk_tensor header;
 *   padding di zeri fino a data_offset (64 bytes per il progetto);
 *   float data[t->size];  // in ordine row-major.
 *
 * Ritorna 1 se tutto ok, 0 in caso di errore.
 */
int tensor_save_to_file(const tensor *t, const char *filename) {
    if (!t) {
        return 0;
    }

    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("tensor_save_to_file: fopen");
        return 0;
    }

    on_disk_tensor header;
    /* Azzero l'header per sicurezza. */
    memset(&header, 0, sizeof(header));

    /* Copio ndim e shape dal tensore in RAM. */
    header.ndim = t->ndim;
    for (int i = 0; i < t->ndim; ++i) {
        header.shape[i] = t->shape[i];
    }

    /* Come da specifiche, data_offset deve essere 64. */
    header.data_offset = 64;

    /* Scrivo l'header all'inizio del file. */
    if (fwrite(&header, sizeof(on_disk_tensor), 1, f) != 1) {
        fclose(f);
        return 0;
    }

    /* Se l'header è più corto di data_offset, aggiungo padding di zeri
     * fino a raggiungere esattamente data_offset bytes.
     */
    long header_size = (long)sizeof(on_disk_tensor);
    long padding = header.data_offset - header_size;
    for (long i = 0; i < padding; ++i) {
        fputc(0, f);
    }

    /* Scrivo tutti i dati del tensore come float consecutivi. */
    size_t n = t->size;
    size_t written = fwrite(t->data, sizeof(float), n, f);

    fclose(f);

    return (written == n) ? 1 : 0;
}
