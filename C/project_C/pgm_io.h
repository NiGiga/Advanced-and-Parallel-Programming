/* Nicola Gigante - SM3201239
 * pgm_io.h
 * Funzioni di I/O per immagini PGM (P5, scala di grigi) e tensori 2D.
 */

#ifndef PGM_IO_H
#define PGM_IO_H

#include "tensor.h"

/* Legge un'immagine PGM (formato binario P5) dal file indicato da filename
 * e la converte in un tensore 2D di float con valori in [0,1].
 * Ritorna NULL in caso di errore (file non apribile, formato non valido, ecc.).
 */
tensor *tensor_read_pgm(const char *filename);

/* Scrive un tensore 2D come immagine PGM (P5, scala di grigi).
 * I valori del tensore vengono:
 * - clampati a [0,1]
 * - rimappati a [0,255] e salvati come unsigned char.
 * Ritorna 0 in caso di errore, 1 se tutto ok.
 */
int tensor_write_pgm(const tensor *img, const char *filename);

#endif /* PGM_IO_H */
