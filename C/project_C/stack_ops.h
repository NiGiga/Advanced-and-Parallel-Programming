/* Nicola Gigante - SM3201239
 * stack_ops.h
 * Operazioni di manipolazione dello stack per TensorForth (d, s, o, D).
 */

#ifndef STACK_OPS_H
#define STACK_OPS_H

#include "stack.h"

/* Implementa l'operazione 'd' (dup).
 * Effetto sullo stack: (a -- a a)
 * Duplica l'elemento in cima allo stack senza copiare i dati del tensore:
 * aumenta solo il refcount in caso di VALUE_TENSOR.
 * Ritorna 1 se tutto ok, 0 in caso di errore (es. stack vuoto).
 */
int op_dup(stack *st);

/* Implementa l'operazione 's' (swap).
 * Effetto sullo stack: (b a -- a b)
 * Scambia i due elementi in cima allo stack.
 * Ritorna 1 se tutto ok, 0 se non ci sono almeno due elementi.
 */
int op_swap(stack *st);

/* Implementa l'operazione 'o' (over).
 * Effetto sullo stack: (b a -- b a b)
 * Duplica il secondo elemento dall'alto e lo mette in cima.
 * Se il valore è un tensore, incrementa il refcount.
 * Ritorna 1 se tutto ok, 0 se non ci sono almeno due elementi.
 */
int op_over(stack *st);

/* Implementa l'operazione 'D' (drop).
 * Effetto sullo stack: (a -- )
 * Rimuove l'elemento in cima allo stack e rilascia la risorsa associata:
 * - se è un tensore, decrementa il refcount (tensor_dec_ref)
 * - se è una stringa, fa free()
 * Ritorna 1 se tutto ok, 0 se lo stack è vuoto.
 */
int op_drop(stack *st);

#endif /* STACK_OPS_H */
