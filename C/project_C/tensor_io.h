/* Nicola Gigante - SM3201239
 * tensor_io.h
 * I/O binario per i tensori nel formato on_disk_tensor, con mmap in lettura.
 */

#ifndef TENSOR_IO_H
#define TENSOR_IO_H

#include "tensor.h"

/* Carica un tensore da file nel formato on_disk_tensor usando mmap.
 * - filename: percorso del file
 * Ritorna un puntatore a tensor che contiene metadati (shape, ndim, size) e
 * un buffer dati che punta dentro la mappatura di memoria.
 * NOTA: per semplicità, NON si gestisce qui la lifetime della mappatura: i dati
 * vengono copiati in un buffer allocato e la mappatura viene rilasciata subito.
 */
tensor *tensor_load_from_file_mmap(const char *filename);

/* Salva un tensore sul disco nel formato on_disk_tensor + dati float, con
 * header allineato a 64 bytes.
 * Ritorna 1 se tutto ok, 0 in caso di errore.
 */
int tensor_save_to_file(const tensor *t, const char *filename);

#endif /* TENSOR_IO_H */
