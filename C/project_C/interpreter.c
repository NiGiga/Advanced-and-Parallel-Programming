/* Nicola Gigante - SM3201239
 * interpreter.c
 * Implementazione dell'esecuzione di singoli token TensorForth.
 */

#include "interpreter.h"
#include "stack_ops.h"
#include "tensor.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

/* Funzione di utilità:
 * Salta eventuali spazi all'inizio di una stringa (non distruttiva).
 */
static const char *skip_spaces(const char *s) {
    while (*s && isspace((unsigned char)*s)) {
        s++;
    }
    return s;
}

/* Parsing di una stringa tra doppi apici, ad esempio: "file.pgm".
 * Alloca una nuova stringa (senza i doppi apici) che poi verrà messa sullo stack.
 * Ritorna NULL in caso di errore di sintassi o memoria.
 */
static char *parse_string_token(const char *tok) {
    size_t len = strlen(tok);
    if (len < 2) {
        return NULL;
    }
    if (tok[0] != '"' || tok[len - 1] != '"') {
        return NULL;  /* Non è una stringa valida. */
    }

    /* Lunghezza del contenuto interno, escludendo i doppi apici. */
    size_t inner_len = len - 2;

    char *s = malloc(inner_len + 1);
    if (!s) {
        return NULL;
    }

    /* Copio il contenuto senza i doppi apici. */
    memcpy(s, tok + 1, inner_len);
    s[inner_len] = '\0';

    return s;
}

/* Parsing di un tensore 1D in notazione [n1 n2 n3 ...].
 * - tok è una stringa che inizia con '[' e finisce con ']'.
 * - Alloca un tensor* con tutti i valori letti come float.
 * Ritorna NULL in caso di errore di sintassi o memoria.
 *
 */
static tensor *parse_tensor_1d(const char *tok) {
    size_t len = strlen(tok);
    if (len < 2 || tok[0] != '[' || tok[len - 1] != ']') {
        return NULL;
    }

    /* Copio il contenuto interno in un buffer modificabile. */
    size_t inner_len = len - 2;
    char *buffer = malloc(inner_len + 1);
    if (!buffer) {
        return NULL;
    }
    memcpy(buffer, tok + 1, inner_len);
    buffer[inner_len] = '\0';

    /* Primo passaggio: conto quanti numeri ci sono. */
    int32_t count = 0;
    char *p = buffer;
    p = (char *)skip_spaces(p);

    while (*p != '\0') {
        char *endptr = NULL;
        strtof(p, &endptr);
        if (endptr == p) {
            /* Nessun numero trovato: sintassi non valida. */
            free(buffer);
            return NULL;
        }
        count++;

        p = (char *)skip_spaces(endptr);
    }

    if (count <= 0) {
        free(buffer);
        return NULL;
    }

    /* Creo il tensore 1D con shape [count].  */
    int32_t shape[1];
    shape[0] = count;
    tensor *t = tensor_create(shape, 1);
    if (!t) {
        fprintf(stderr, "DEBUG parse_tensor_1d: tensor_create fallita (count=%d)\n", count);
        free(buffer);
        return NULL;
    }

    /* Secondo passaggio: reinizializzo p e ri-leggo i numeri, riempiendo t->data. */
    p = buffer;
    p = (char *)skip_spaces(p);
    for (int32_t i = 0; i < count; ++i) {
        char *endptr = NULL;
        float val = strtof(p, &endptr);
        t->data[i] = val;
        p = (char *)skip_spaces(endptr);
    }

    free(buffer);
    return t;
}

/* Esegue un singolo token sullo stack. */
interpreter_status execute_token(stack *st, const char *tok) {
    if (!st || !tok) {
        return INTERPRETER_PARSE_ERROR;
    }

    /* Salto eventuali spazi (anche se idealmente tok è già un singolo token). */
    tok = skip_spaces(tok);
    if (*tok == '\0') {
        return INTERPRETER_OK;  /* Token vuoto: non faccio nulla. */
    }

    /* 1) Token di una sola lettera per le operazioni di stack. */
    if (tok[1] == '\0') {  /* stringa di lunghezza 1 */
        switch (tok[0]) {
            case 'd':
                if (!op_dup(st)) {
                    return INTERPRETER_STACK_ERROR;
                }
                return INTERPRETER_OK;

            case 's':
                if (!op_swap(st)) {
                    return INTERPRETER_STACK_ERROR;
                }
                return INTERPRETER_OK;

            case 'o':
                if (!op_over(st)) {
                    return INTERPRETER_STACK_ERROR;
                }
                return INTERPRETER_OK;

            case 'D':
                if (!op_drop(st)) {
                    return INTERPRETER_STACK_ERROR;
                }
                return INTERPRETER_OK;

            case 'p': {
                /* p(a--) stampa il tensore e lo consuma. */
                value v;
                if (!stack_pop(st, &v)) {
                    return INTERPRETER_STACK_ERROR;
                }

                if (v.type != VALUE_TENSOR || v.as.t == NULL) {
                    /* Tipo sbagliato: mi aspettavo un tensore. */
                    /* Libero comunque la risorsa se è una stringa. */
                    if (v.type == VALUE_STRING && v.as.s != NULL) {
                        free(v.as.s);
                    }
                    return INTERPRETER_TYPE_ERROR;
                }

                /* Stampo il tensore e poi rilascio il riferimento. */
                tensor_print(v.as.t);
                tensor_dec_ref(v.as.t);
                return INTERPRETER_OK;
            }
	    case 'R': {
                /* R(a -- relu(a)). [file:1] */
                value va;
                if (!stack_pop(st, &va)) {
                    return INTERPRETER_STACK_ERROR;
                }

                if (va.type != VALUE_TENSOR || !va.as.t) {
                    if (va.type == VALUE_STRING && va.as.s) {
                        free(va.as.s);
                    }
                    return INTERPRETER_TYPE_ERROR;
                }

                tensor *res = tensor_relu(va.as.t);
                tensor_dec_ref(va.as.t);

                if (!res) {
                    return INTERPRETER_MEMORY_ERROR;
                }

                value vres;
                vres.type = VALUE_TENSOR;
                vres.as.t = res;

                if (!stack_push(st, vres)) {
                    tensor_dec_ref(res);
                    return INTERPRETER_MEMORY_ERROR;
                }

                return INTERPRETER_OK;
            }
	    case '#': {
                /* #(a -- #a): restituisce la shape di a come tensore 1D. */
                value va;
                if (!stack_pop(st, &va)) {
                    return INTERPRETER_STACK_ERROR;
                }

                if (va.type != VALUE_TENSOR || !va.as.t) {
                    if (va.type == VALUE_STRING && va.as.s) {
                        free(va.as.s);
                    }
                    return INTERPRETER_TYPE_ERROR;
                }

                tensor *res = tensor_shape_tensor(va.as.t);
                tensor_dec_ref(va.as.t);

                if (!res) {
                    return INTERPRETER_MEMORY_ERROR;
                }

                value vres;
                vres.type = VALUE_TENSOR;
                vres.as.t = res;

                if (!stack_push(st, vres)) {
                    tensor_dec_ref(res);
                    return INTERPRETER_MEMORY_ERROR;
                }

                return INTERPRETER_OK;
            }
	    case '_': {
                /* _(a -- a'): ravel, vista 1D del tensore a. */
                value va;
                if (!stack_pop(st, &va)) {
                    return INTERPRETER_STACK_ERROR;
                }

                if (va.type != VALUE_TENSOR || !va.as.t) {
                    if (va.type == VALUE_STRING && va.as.s) {
                        free(va.as.s);
                    }
                    return INTERPRETER_TYPE_ERROR;
                }

                tensor *res = tensor_ravel(va.as.t);
                /* Decremento il riferimento all'originale, tengo solo la vista. */
                tensor_dec_ref(va.as.t);

                if (!res) {
                    return INTERPRETER_MEMORY_ERROR;
                }

                value vres;
                vres.type = VALUE_TENSOR;
                vres.as.t = res;

                if (!stack_push(st, vres)) {
                    tensor_dec_ref(res);
                    return INTERPRETER_MEMORY_ERROR;
                }

                return INTERPRETER_OK;
            }
	    case 'r': {
	      value vs, va;

	      if (!stack_pop(st, &vs) || !stack_pop(st, &va)) {
		if (vs.type == VALUE_TENSOR && vs.as.t) tensor_dec_ref(vs.as.t);
		if (vs.type == VALUE_STRING && vs.as.s) free(vs.as.s);
		if (va.type == VALUE_TENSOR && va.as.t) tensor_dec_ref(va.as.t);
		if (va.type == VALUE_STRING && va.as.s) free(va.as.s);
		return INTERPRETER_STACK_ERROR;
	      }

	      printf("DEBUG r: va.type=%d va.as.t=%p, vs.type=%d vs.as.t=%p\n",
		     va.type, (void*)va.as.t, vs.type, (void*)vs.as.t);

	      /* Controllo SOLO i tipi. */
	      if (va.type != VALUE_TENSOR || vs.type != VALUE_TENSOR) {
		if (va.type == VALUE_TENSOR && va.as.t) tensor_dec_ref(va.as.t);
		if (va.type == VALUE_STRING && va.as.s) free(va.as.s);
		if (vs.type == VALUE_TENSOR && vs.as.t) tensor_dec_ref(vs.as.t);
		if (vs.type == VALUE_STRING && vs.as.s) free(vs.as.s);
		return INTERPRETER_TYPE_ERROR;
	      }

	      tensor *res = tensor_reshape(va.as.t, vs.as.t);
	      tensor_dec_ref(va.as.t);
	      tensor_dec_ref(vs.as.t);

	      if (!res) {
		fprintf(stderr, "DEBUG r: tensor_reshape ha restituito NULL\n");
		return INTERPRETER_TYPE_ERROR; /* si potrebbe implementare un  errore di shape. */
	      }

	      value vres;
	      vres.type = VALUE_TENSOR;
	      vres.as.t = res;

	      if (!stack_push(st, vres)) {
		tensor_dec_ref(res);
		return INTERPRETER_MEMORY_ERROR;
	      }

	      return INTERPRETER_OK;
	    }

            default:
                /* Non è una delle stack-ops, continuo con gli altri casi. */
                break;
        }
    }

    /* 2) Stringhe tra doppi apici: "file.pgm" */
    if (tok[0] == '"') {
        char *s = parse_string_token(tok);
        if (!s) {
            return INTERPRETER_PARSE_ERROR;
        }

        value v;
        v.type = VALUE_STRING;
        v.as.s = s;

        if (!stack_push(st, v)) {
            free(s);
            return INTERPRETER_MEMORY_ERROR;
        }

        return INTERPRETER_OK;
    }

    /* 3) Tensore 1D tra parentesi quadre: [1 2 3] */
    if (tok[0] == '[') {
        tensor *t = parse_tensor_1d(tok);
        if (!t) {
            return INTERPRETER_PARSE_ERROR;
        }

        value v;
        v.type = VALUE_TENSOR;
        v.as.t = t;

        if (!stack_push(st, v)) {
            /* Se il push fallisce, rilascio il tensore. */
            tensor_dec_ref(t);
            return INTERPRETER_MEMORY_ERROR;
        }

        return INTERPRETER_OK;
    }

    /* 4) Operatori aritmetici elementwise: +, -, * (b a -- a op b). */
    if ((tok[0] == '+' || tok[0] == '-' || tok[0] == '*') && tok[1] == '\0') {
        char op = tok[0];
        value va, vb;

        /* Pop a (cima) e poi b (secondo). */
        if (!stack_pop(st, &va) || !stack_pop(st, &vb)) {
            if (va.type == VALUE_TENSOR && va.as.t) tensor_dec_ref(va.as.t);
            if (va.type == VALUE_STRING && va.as.s) free(va.as.s);
            if (vb.type == VALUE_TENSOR && vb.as.t) tensor_dec_ref(vb.as.t);
            if (vb.type == VALUE_STRING && vb.as.s) free(vb.as.s);
            return INTERPRETER_STACK_ERROR;
        }

        /* Mi aspetto due tensori. */
        if (va.type != VALUE_TENSOR || vb.type != VALUE_TENSOR ||
            !va.as.t || !vb.as.t) {

            if (va.type == VALUE_TENSOR && va.as.t) tensor_dec_ref(va.as.t);
            if (va.type == VALUE_STRING && va.as.s) free(va.as.s);
            if (vb.type == VALUE_TENSOR && vb.as.t) tensor_dec_ref(vb.as.t);
            if (vb.type == VALUE_STRING && vb.as.s) free(vb.as.s);

            return INTERPRETER_TYPE_ERROR;
        }

        /* Scelgo l'operazione giusta. */
        tensor *res = NULL;
        if (op == '+') {
            res = tensor_add(vb.as.t, va.as.t);
        } else if (op == '-') {
            res = tensor_sub(vb.as.t, va.as.t);
        } else if (op == '*') {
            res = tensor_mul(vb.as.t, va.as.t);
        }

        /* Dopo l'uso, rilascio a e b. */
        tensor_dec_ref(va.as.t);
        tensor_dec_ref(vb.as.t);

        if (!res) {
            return INTERPRETER_TYPE_ERROR;  /* shape incompatibile o memoria. */
        }

        value vres;
        vres.type = VALUE_TENSOR;
        vres.as.t = res;

        if (!stack_push(st, vres)) {
            tensor_dec_ref(res);
            return INTERPRETER_MEMORY_ERROR;
        }

        return INTERPRETER_OK;
    }

    /* Se arrivo qui, il token non è ancora gestito.
     * In futuro qui aggiungerò -, *, @, R, ecc.
     */
    return INTERPRETER_PARSE_ERROR;
}
