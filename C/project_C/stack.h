/* Nicola Gigante - SM3201239
 * stack.h
 * Implementazione di uno stack generico per l'interprete TensorForth.
 */

#ifndef STACK_H
#define STACK_H

#include "tensor.h"

/* Tipo di dato che può stare sullo stack:
 * - TENSOR: puntatore a struct tensor
 * - STRING: puntatore a char (filename tra doppi apici)
 */
typedef enum {
    VALUE_TENSOR,
    VALUE_STRING
} value_type;

/* Valore generico sullo stack. */
typedef struct {
    value_type type;  /* Indica cosa contiene questo elemento. */
    union {
        tensor *t;    /* Puntatore a tensore (quando type == VALUE_TENSOR). */
        char   *s;    /* Puntatore a stringa (quando type == VALUE_STRING). */
    } as;
} value;

/* Struttura dello stack vero e proprio. */
typedef struct {
    value *data;   /* Array dinamico di elementi. */
    int    top;    /* Indice dell'elemento in cima (prossima posizione libera). */
    int    capacity; /* Dimensione massima corrente dell'array. */
} stack;

/* Crea uno stack vuoto con una capacità iniziale. */
stack *stack_create(int initial_capacity);

/* Libera lo stack e tutti i valori che possiede.
 * NB: per i tensori decrementiamo il refcount, per le stringhe facciamo free().
 */
void stack_destroy(stack *st);

/* Ritorna 1 se lo stack è vuoto, 0 altrimenti. */
int stack_is_empty(const stack *st);

/* Inserisce un valore in cima allo stack.
 * Ritorna 0 in caso di errore (ad esempio realloc fallita), 1 altrimenti.
 */
int stack_push(stack *st, value v);

/* Rimuove e ritorna l'elemento in cima allo stack.
 * In caso di stack vuoto, ritorna 0 e non modifica *out.
 */
int stack_pop(stack *st, value *out);

/* Legge (senza rimuoverlo) l'elemento in cima allo stack.
 * In caso di stack vuoto, ritorna 0.
 */
int stack_peek(const stack *st, value *out);

#endif /* STACK_H */
