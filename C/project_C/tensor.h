/* Nicola Gigante - SM3201239
 * tensor.h
 * Definizione della struttura tensor e delle funzioni base di gestione.
 */

#ifndef TENSOR_H
#define TENSOR_H

#include <stdint.h>
#include <stddef.h>
#include <sys/types.h>  /* per off_t (usato in on_disk_tensor). */

#define MAX_DIM 2  /* Nel progetto lavoriamo solo con tensori 1D/2D. */

/* Struttura usata per salvare/caricare i tensori su disco. */
typedef struct {
    int32_t shape[MAX_DIM];
    int32_t ndim;
    off_t   data_offset;  /* Offset dall'inizio del file, allineato a 64 byte. */
} on_disk_tensor;

/* Tipo di dato in RAM che rappresenta un tensore.
 * Qui teniamo anche un contatore di riferimenti per gestire dup/over senza copie.
 */
typedef struct {
    float  *data;             /* Puntatore ai dati contigui in memoria. */
    int32_t shape[MAX_DIM];   /* Dimensioni correnti del tensore (1D o 2D).*/
    int32_t ndim;             /* Quante dimensioni sono effettivamente usate (1 o 2). */
    size_t size;              /* Numero totale di elementi = prodotto delle dimensioni. */
    int    refcount;          /* Numero di riferimenti logici a questo tensore. */
    /* Campi per gestire dati mappati con mmap. */
    void  *mmap_base;   /* indirizzo base della mappatura, oppure NULL se non mappato */
    size_t mmap_size;   /* dimensione della mappatura in bytes */
} tensor;

/* Crea un nuovo tensore con la shape indicata (shape[0..ndim-1]).
 * Alloca i dati (inizialmente non inizializzati) e imposta refcount = 1.
 * Ritorna NULL in caso di errore di allocazione.
 */
tensor *tensor_create(int32_t *shape, int32_t ndim);

/* Aggiunge un riferimento al tensore (usato da dup/over dello stack). */
void tensor_inc_ref(tensor *t);

/* Rimuove un riferimento; se refcount arriva a 0 libera il tensore. */
void tensor_dec_ref(tensor *t);

/* Funzione di utilità per calcolare il numero totale di elementi dati shape e ndim.
 * Ritorna 0 in caso di overflow o input non valido.
 */
size_t tensor_num_elements(const int32_t *shape, int32_t ndim);

/* Funzione di comodo per controllare se due tensori hanno stessa shape e ndim. */
int tensor_same_shape(const tensor *a, const tensor *b);

/* Stampa un tensore nel formato:
 * Tensor(shape=[d1 d2], data=[v1 v2 ...])
 * Non modifica il tensore.
 */
void tensor_print(const tensor *t);

/* Somma elementwise tra due tensori con stessa shape.
 * - a, b: tensori di input (non modificati)
 * - ritorna un nuovo tensore con i dati a[i] + b[i]
 *   oppure NULL in caso di errore (shape diversa o memoria).
 */
tensor *tensor_add(const tensor *a, const tensor *b);

/* Differenza elementwise: res = a - b (stessa shape). */
tensor *tensor_sub(const tensor *a, const tensor *b);

/* Prodotto elementwise: res = a * b (stessa shape). */
tensor *tensor_mul(const tensor *a, const tensor *b);

/* ReLU elementwise: res[i] = max(0, a[i]). */
tensor *tensor_relu(const tensor *a);

/* MIN: m (b a -- min(a,b)): per ogni elemento prende il minimo tra a[i] e b[i]. */
tensor *tensor_min(const tensor *a, const tensor *b);

/* MAX: M (b a -- max(a,b)): per ogni elemento prende il massimo. */
tensor *tensor_max(const tensor *a, const tensor *b);

/* Confronto elementwise: res = (a < b) ? 1.0 : 0.0 (stessa shape). */
tensor *tensor_lt(const tensor *a, const tensor *b);

/* Confronto elementwise: res = (a > b) ? 1.0 : 0.0. */
tensor *tensor_gt(const tensor *a, const tensor *b);

/* Uguaglianza elementwise: res = (a == b) ? 1.0 : 0.0. */
tensor *tensor_eq(const tensor *a, const tensor *b);

/* AND */
tensor *tensor_and(const tensor *a, const tensor *b);

/* OR */
tensor *tensor_or(const tensor *a, const tensor *b);

/* NOT */
tensor *tensor_not(const tensor *a);

/* Restituisce la shape del tensore come tensore 1D di 1 o 2 elementi. */
tensor *tensor_shape_tensor(const tensor *a);

/* Ravel: restituisce una vista 1D dei dati di a, senza ricopiare i dati. */
tensor *tensor_ravel(tensor *a);

/* Reshape: cambia ndim/shape del tensore a in base al tensore 1D s (1 o 2 dims).
 * Non cambia l'ordine dei dati in memoria.
 */
tensor *tensor_reshape(tensor *a, const tensor *s);

/* Riduzione: somma di tutti gli elementi di a in un tensore 1D di 1 elemento. */
tensor *tensor_sum_all(const tensor *a);

/* Genera un tensore di shape definita da s (1D di 1 o 2 elementi), 
 * riempito con float random uniformi in [0,1]. */
tensor *tensor_random_from_shape(const tensor *s);

/* Fill: crea un tensore di shape s e lo riempie ripetendo i valori di v. */
tensor *tensor_fill_from_shape(const tensor *s, const tensor *v);

/* Prodotto di matrici 2D: se a è m×k e b è k×n, ritorna m×n. */
tensor *tensor_matmul(const tensor *a, const tensor *b);

/* Prodotto scalare tra due vettori 1D di stessa dimensione. */
tensor *tensor_dot(const tensor *a, const tensor *b);

/* Convoluzione 2D con stride 1 e padding di zeri: 
 * a: input H×W, k: kernel Kh×Kw, output H×W. */
tensor *tensor_conv2d(const tensor *a, const tensor *k);

#endif /* TENSOR_H */
