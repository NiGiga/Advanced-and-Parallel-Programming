/* Nicola Gigante - SM3201239
 * stack.c
 * Implementazione dello stack generico usato dall'interprete TensorForth.
 */

#include "stack.h"
#include <stdlib.h>

/* Funzione di utilità interna: raddoppia la capacità dello stack quando pieno.
 * Ritorna 1 in caso di successo, 0 in caso di errore (realloc fallita).
 */
static int stack_grow(stack *st) {
    int new_capacity = (st->capacity <= 0) ? 8 : st->capacity * 2;

    value *new_data = realloc(st->data, new_capacity * sizeof(value));
    if (!new_data) {
        return 0;
    }

    st->data = new_data;
    st->capacity = new_capacity;
    return 1;
}

/* Crea uno stack vuoto con una capacità iniziale.
 * Se initial_capacity <= 0, uso una capacità di default.
 */
stack *stack_create(int initial_capacity) {
    stack *st = malloc(sizeof(stack));
    if (!st) {
        return NULL;
    }

    if (initial_capacity <= 0) {
        initial_capacity = 8;  /* Valore di default per non riallocare subito. */
    }

    st->data = malloc(initial_capacity * sizeof(value));
    if (!st->data) {
        free(st);
        return NULL;
    }

    st->top = 0;              /* Nessun elemento nello stack all'inizio. */
    st->capacity = initial_capacity;

    return st;
}

/* Libera tutti gli elementi presenti nello stack e poi la struttura stessa.
 * Per i tensori decremento il refcount; se arriva a 0 verranno liberati.
 * Per le stringhe, assumo che siano state allocate con malloc e le rilascio.
 */
void stack_destroy(stack *st) {
    if (!st) {
        return;
    }

    /* Scorro tutti gli elementi ancora presenti nello stack. */
    for (int i = 0; i < st->top; ++i) {
        value *v = &st->data[i];
        if (v->type == VALUE_TENSOR) {
            /* Rimuovo un riferimento logico al tensore. */
            tensor_dec_ref(v->as.t);
        } else if (v->type == VALUE_STRING) {
            /* Libero la stringa associata (es. filename). */
            free(v->as.s);
        }
    }

    free(st->data);
    free(st);
}

/* Ritorna 1 se lo stack è vuoto, 0 altrimenti. */
int stack_is_empty(const stack *st) {
    if (!st) {
        return 1;
    }
    return (st->top == 0);
}

/* Inserisce un valore in cima allo stack.
 * NOTA: qui non modifichiamo il refcount del tensore; lo faremo
 *       solo quando eseguiamo operazioni come dup/over.
 */
int stack_push(stack *st, value v) {
    if (!st) {
        return 0;
    }

    /* Se non c'è spazio, provo ad aumentare la capacità. */
    if (st->top >= st->capacity) {
        if (!stack_grow(st)) {
            return 0;
        }
    }

    /* Copio il value nella posizione top e avanzo il puntatore. */
    st->data[st->top] = v;
    st->top += 1;

    return 1;
}

/* Rimuove e ritorna l'elemento in cima allo stack.
 * Non tocca la memoria interna (tensore/stringa): la responsabilità
 * di gestire refcount/free è di chi usa questo valore.
 */
int stack_pop(stack *st, value *out) {
    if (!st || st->top <= 0) {
        return 0;  /* Stack vuoto o puntatore non valido. */
    }

    st->top -= 1;
    if (out) {
        *out = st->data[st->top];
    }

    return 1;
}

/* Legge (senza rimuoverlo) l'elemento in cima allo stack. */
int stack_peek(const stack *st, value *out) {
    if (!st || st->top <= 0) {
        return 0;  /* Stack vuoto o puntatore non valido. */
    }

    if (out) {
        *out = st->data[st->top - 1];
    }

    return 1;
}
