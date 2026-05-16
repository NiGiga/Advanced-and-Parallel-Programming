/* Nicola Gigante - SM3201239
 * interpreter.h
 * Interprete per il linguaggio TensorForth: parsing ed esecuzione dei token.
 */

#ifndef INTERPRETER_H
#define INTERPRETER_H

#include "stack.h"

/* Tipo di errore generico usato dall'interprete. */
typedef enum {
    INTERPRETER_OK = 0,          /* Nessun errore. */
    INTERPRETER_STACK_ERROR,     /* Problema di stack (underflow, ecc.). */
    INTERPRETER_PARSE_ERROR,     /* Token non valido o sintassi errata. */
    INTERPRETER_TYPE_ERROR,      /* Tipo di operando non atteso. */
    INTERPRETER_MEMORY_ERROR     /* Errore di allocazione. */
} interpreter_status;

/* Esegue un singolo token sullo stack.
 * - st: stack dei valori su cui operano i token.
 * - tok: stringa che contiene il token già separato (senza spazi).
 *
 * Ritorna un codice di stato che indica se l'operazione è andata a buon fine
 * oppure quale tipo di errore si è verificato.
 */
interpreter_status execute_token(stack *st, const char *tok);

#endif /* INTERPRETER_H */
