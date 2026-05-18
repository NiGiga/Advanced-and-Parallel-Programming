#include "pgm_io.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Leggo un file PGM (formato P5) e lo converto in un tensore 2D di float
 * con valori normalizzati in [0,1]. In caso di errore ritorno NULL. */
tensor *tensor_read_pgm(const char *filename) {
    /* Apro il file in binario; se fallisce, segnalo l’errore di sistema. */
    FILE *f = fopen(filename, "rb");
    if (!f) {
        perror("tensor_read_pgm: fopen");
        return NULL;
    }

    /* Leggo la “magic string” del formato PGM, mi aspetto “P5”. */
    char magic[3] = {0};
    if (fscanf(f, "%2s", magic) != 1) {
        fclose(f);
        return NULL;
    }
    if (strcmp(magic, "P5") != 0) {
        /* Non è un PGM binario nella forma attesa. */
        fclose(f);
        return NULL;
    }

    int width, height, maxval;

    /* Leggo larghezza e altezza dell’immagine. */
    if (fscanf(f, "%d %d", &width, &height) != 2) {
        fclose(f);
        return NULL;
    }

    /* Leggo il valore massimo (di solito 255). */
    if (fscanf(f, "%d", &maxval) != 1) {
        fclose(f);
        return NULL;
    }
    /* Verifico che maxval abbia un valore sensato. */
    if (maxval <= 0 || maxval > 65535) {
        fclose(f);
        return NULL;
    }

    /* Dopo maxval c’è un carattere di newline che devo consumare
     * prima di leggere i dati binari. */
    fgetc(f);

    /* Costruisco la shape del tensore 2D: [height, width]. */
    int32_t shape[2] = { height, width };
    tensor *img = tensor_create(shape, 2);
    if (!img) {
        fclose(f);
        return NULL;
    }

    /* Numero totale di pixel dell’immagine. */
    size_t n = (size_t)width * (size_t)height;

    /* Buffer temporaneo per leggere i byte grezzi dell’immagine. */
    unsigned char *buffer = malloc(n);
    if (!buffer) {
        tensor_dec_ref(img);
        fclose(f);
        return NULL;
    }

    /* Leggo tutti i pixel in un colpo solo. */
    size_t read_n = fread(buffer, 1, n, f);
    fclose(f);
    if (read_n != n) {
        /* Se non riesco a leggere tutti i pixel, considero l’operazione fallita. */
        free(buffer);
        tensor_dec_ref(img);
        return NULL;
    }

    /* Trasformo ogni byte in un float in [0,1], usando maxval. */
    for (size_t i = 0; i < n; ++i) {
        img->data[i] = (float)buffer[i] / (float)maxval;
    }

    free(buffer);
    return img;
}

/* Scrivo un tensore 2D come immagine PGM (P5, scala di grigi).
 * I valori del tensore vengono clampati in [0,1] e convertiti in [0,255]. */
int tensor_write_pgm(const tensor *img, const char *filename) {
    /* Mi aspetto un tensore 2D; se non lo è, non posso salvarlo come immagine. */
    if (!img || img->ndim != 2) {
        return 0;
    }

    int height = img->shape[0];
    int width  = img->shape[1];
    int maxval = 255;

    /* Apro il file in scrittura binaria. */
    FILE *f = fopen(filename, "wb");
    if (!f) {
        perror("tensor_write_pgm: fopen");
        return 0;
    }

    /* Scrivo l’intestazione PGM: magic, dimensioni, maxval. */
    fprintf(f, "P5\n%d %d\n%d\n", width, height, maxval);

    size_t n = (size_t)width * (size_t)height;

    /* Buffer temporaneo per i byte da scrivere sul file. */
    unsigned char *buffer = malloc(n);
    if (!buffer) {
        fclose(f);
        return 0;
    }

    /* Converto ogni valore del tensore in un byte [0,255]:
     * prima clamp in [0,1], poi scala con maxval. */
    for (size_t i = 0; i < n; ++i) {
        float x = img->data[i];
        if (x < 0.0f) x = 0.0f;
        if (x > 1.0f) x = 1.0f;
        buffer[i] = (unsigned char)(x * (float)maxval + 0.5f);
    }

    /* Scrivo tutti i pixel. */
    size_t written = fwrite(buffer, 1, n, f);
    free(buffer);
    fclose(f);

    /* Ritorno 1 solo se sono riuscito a scrivere tutti i dati. */
    return (written == n) ? 1 : 0;
}
