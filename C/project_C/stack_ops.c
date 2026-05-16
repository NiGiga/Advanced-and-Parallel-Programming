/* Nicola Gigante - SM3201239
 * stack_ops.c
 * Implementazione delle operazioni di manipolazione dello stack
 * corrispondenti ai token d, s, o, D di TensorForth.
 */

#include "stack_ops.h"
#include <stdlib.h>

/* Duplica l'elemento in cima allo stack.
 * Non crea una nuova copia del tensore: aumenta solo il refcount.
 */
int op_dup(stack *st) {
    if (!st || st->top <= 0) {
        return 0;  /* Errore: stack vuoto. */
    }

    /* Leggo il valore in cima. */
    value v = st->data[st->top - 1];

    /* Se è un tensore, incremento il refcount perché avrò un nuovo riferimento logico. */
    if (v.type == VALUE_TENSOR && v.as.t != NULL) {
        tensor_inc_ref(v.as.t);
    }

    /* Ora pusho una copia del value (stesso puntatore). */
    if (!stack_push(st, v)) {
        /* Se il push fallisce, devo annullare l'incremento di refcount. */
        if (v.type == VALUE_TENSOR && v.as.t != NULL) {
            tensor_dec_ref(v.as.t);
        }
        return 0;
    }

    return 1;
}

/* Scambia i due elementi in cima allo stack. */
int op_swap(stack *st) {
    if (!st || st->top < 2) {
        return 0;  /* Errore: non ci sono almeno due elementi. */
    }

    value *a = &st->data[st->top - 1]; /* elemento in cima */
    value *b = &st->data[st->top - 2]; /* secondo elemento */

    value tmp = *a;
    *a = *b;
    *b = tmp;

    return 1;
}

/* Duplica il secondo elemento dall'alto e lo mette in cima.  */
int op_over(stack *st) {
    if (!st || st->top < 2) {
        return 0;  /* Errore: non ci sono almeno due elementi. */
    }

    /* Il secondo elemento dall'alto è in posizione top - 2. */
    value v = st->data[st->top - 2];

    /* Se è un tensore, incremento il refcount per il nuovo riferimento. */
    if (v.type == VALUE_TENSOR && v.as.t != NULL) {
        tensor_inc_ref(v.as.t);
    }

    if (!stack_push(st, v)) {
        /* Se il push fallisce, annullo l'incremento del refcount. */
        if (v.type == VALUE_TENSOR && v.as.t != NULL) {
            tensor_dec_ref(v.as.t);
        }
        return 0;
    }

    return 1;
}

/* Rimuove l'elemento in cima allo stack e libera la risorsa associata. */
int op_drop(stack *st) {
    if (!st || st->top <= 0) {
        return 0;  /* Errore: stack vuoto. */
    }

    /* Pop dal top. Uso stack_pop per mantenere una sola logica di modifica del top. */
    value v;
    if (!stack_pop(st, &v)) {
        return 0;
    }

    /* Rilascio la risorsa corrispondente al valore. */
    if (v.type == VALUE_TENSOR && v.as.t != NULL) {
        tensor_dec_ref(v.as.t);  /* Se refcount arriva a 0, il tensore viene liberato. */
    } else if (v.type == VALUE_STRING && v.as.s != NULL) {
        free(v.as.s);           /* Libero la stringa (filename). */
    }

    return 1;
}
