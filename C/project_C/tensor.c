/* Nicola Gigante - SM3201239
 * tensor.c
 * Implementazione delle primitive di gestione dei tensori.
 */

#include "tensor.h"
#include <stdlib.h>
#include <stdio.h>  /* per printf nella tensor_print */
#include <omp.h> 

/* Calcolo del numero totale di elementi a partire dalla shape.
 * Qui assumiamo dimensioni non negative; se ndim è fuori range, ritorniamo 0.
 */
size_t tensor_num_elements(const int32_t *shape, int32_t ndim) {
    if (ndim <= 0 || ndim > MAX_DIM) {
        return 0;
    }

    size_t total = 1;
    for (int32_t i = 0; i < ndim; ++i) {
        if (shape[i] <= 0) {
            return 0;
        }
        total *= (size_t)shape[i];
    }
    return total;
}

/* Crea un tensore con la shape indicata e alloca il buffer dati. */
tensor *tensor_create(int32_t *shape, int32_t ndim) {
    tensor *t = malloc(sizeof(tensor));
    if (!t) {
        return NULL;
    }

    t->ndim = ndim;

    /* Copio le dimensioni nella struttura interna. */
    for (int i = 0; i < MAX_DIM; ++i) {
        if (i < ndim) {
            t->shape[i] = shape[i];
        } else {
            t->shape[i] = 0;  /* Per sicurezza azzero le dimensioni non usate. */
        }
    }

    t->size = tensor_num_elements(t->shape, t->ndim);
    if (t->size == 0) {
        free(t);
        return NULL;
    }

    /* Alloco il buffer dei dati (float). */
    t->data = malloc(t->size * sizeof(float));
    if (!t->data) {
        free(t);
        return NULL;
    }

    t->refcount = 1;  /* Quando creo il tensore, c'è un solo riferimento logico. */

    return t;
}

/* Incrementa il contatore di riferimenti (usato per dup/over). */
void tensor_inc_ref(tensor *t) {
    if (!t) {
        return;
    }
    t->refcount += 1;
}

/* Decrementa il contatore di riferimenti e libera il tensore se arriva a 0. */
void tensor_dec_ref(tensor *t) {
    if (!t) {
        return;
    }

    t->refcount -= 1;
    if (t->refcount <= 0) {
        /* Libero prima il buffer dati, poi la struttura. */
        free(t->data);
        free(t);
    }
}

/* Verifica se due tensori hanno la stessa shape (stesso ndim e stesse dimensioni). */
int tensor_same_shape(const tensor *a, const tensor *b) {
    if (!a || !b) {
        return 0;
    }
    if (a->ndim != b->ndim) {
        return 0;
    }
    for (int i = 0; i < a->ndim; ++i) {
        if (a->shape[i] != b->shape[i]) {
            return 0;
        }
    }
    return 1;
}

/* Stampa il tensore nel formato richiesto dal progetto. */
void tensor_print(const tensor *t) {
    if (!t) {
        printf("Tensor(shape=[], data=[])\n");
        return;
    }

    printf("Tensor(shape=[");

    /* Stampo solo le dimensioni effettive (ndim). */
    for (int i = 0; i < t->ndim; ++i) {
        printf("%d", t->shape[i]);
        if (i + 1 < t->ndim) {
            printf(" ");
        }
    }
    printf("], data=[");

    /* Stampo tutti i valori in ordine lineare. */
    for (size_t i = 0; i < t->size; ++i) {
        printf("%g", t->data[i]);
        if (i + 1 < t->size) {
            printf(" ");
        }
    }

    printf("])\n");
}

tensor *tensor_add(const tensor *a, const tensor *b) {
    if (!a || !b) {
        return NULL;
    }

    /* Controllo che le shape coincidano. */
    if (!tensor_same_shape(a, b)) {
        return NULL;
    }

    /* Creo un nuovo tensore con la stessa shape di a. */
    tensor *res = tensor_create((int32_t *)a->shape, a->ndim);
    if (!res) {
        return NULL;
    }

    /* Somma elementwise: res[i] = a[i] + b[i]. */
    size_t n = a->size;

    /* Uso OpenMP: */  
    #pragma omp parallel for
    for (size_t i = 0; i < n; ++i) {
      res->data[i] = a->data[i] + b->data[i];
    }
     

    return res;
}

tensor *tensor_sub(const tensor *a, const tensor *b) {
    if (!a || !b) {
        return NULL;
    }
    if (!tensor_same_shape(a, b)) {
        return NULL;
    }

    tensor *res = tensor_create((int32_t *)a->shape, a->ndim);
    if (!res) {
        return NULL;
    }

    size_t n = a->size;
    for (size_t i = 0; i < n; ++i) {
        res->data[i] = a->data[i] - b->data[i];
    }
    return res;
}

tensor *tensor_mul(const tensor *a, const tensor *b) {
    if (!a || !b) {
        return NULL;
    }
    if (!tensor_same_shape(a, b)) {
        return NULL;
    }

    tensor *res = tensor_create((int32_t *)a->shape, a->ndim);
    if (!res) {
        return NULL;
    }

    size_t n = a->size;
    for (size_t i = 0; i < n; ++i) {
        res->data[i] = a->data[i] * b->data[i];
    }
    return res;
}

tensor *tensor_relu(const tensor *a) {
    if (!a) {
        return NULL;
    }

    tensor *res = tensor_create((int32_t *)a->shape, a->ndim);
    if (!res) {
        return NULL;
    }

    size_t n = a->size;
    for (size_t i = 0; i < n; ++i) {
        float x = a->data[i];
        res->data[i] = (x > 0.0f) ? x : 0.0f;
    }

    return res;
}

/* Funzione di supporto interna per i confronti elementwise.
 * Confronta due tensori a e b che devono avere la stessa shape.
 * Il parametro mode seleziona l'operazione:
 *   mode = 0  --> confronto av < bv
 *   mode = 1  --> confronto av > bv
 *   mode = 2  --> confronto av == bv
 * Per ogni elemento genera 1.0f se la condizione è vera, altrimenti 0.0f.
 * Ritorna un nuovo tensore con lo stesso shape di a, oppure NULL in caso di errore.
 */
static tensor *tensor_compare_template(const tensor *a, const tensor *b, int mode) {
    /* Controllo che i puntatori di input siano validi. */
    if (!a || !b) {
        return NULL;
    }

    /* I due tensori devono avere esattamente la stessa shape. */
    if (!tensor_same_shape(a, b)) {
        return NULL;
    }

    /* Alloco il tensore risultato con la stessa shape e ndim di a. */
    tensor *res = tensor_create((int32_t *)a->shape, a->ndim);
    if (!res) {
        return NULL;
    }

    /* Numero totale di elementi da confrontare. */
    size_t n = a->size;

    /* Scorro tutti gli elementi in parallelo nei due tensori. */
    for (size_t i = 0; i < n; ++i) {
        float av = a->data[i];  /* elemento i-esimo di a */
        float bv = b->data[i];  /* elemento i-esimo di b */
        int cond = 0;           /* variabile booleana intera (0 = falso, 1 = vero) */

        /* Seleziono il tipo di confronto in base al mode. */
        if (mode == 0) {
            cond = (av < bv);
        } else if (mode == 1) {
            cond = (av > bv);
        } else { /* mode == 2: uguaglianza */
            cond = (av == bv);
        }

        /* Salvo 1.0f se la condizione è vera, altrimenti 0.0f. */
        res->data[i] = cond ? 1.0f : 0.0f;
    }

    return res;
}

/* Confronto elementwise "minore di".
 * Ritorna un tensore di 0.0/1.0 con stessa shape di a e b.
 */
tensor *tensor_lt(const tensor *a, const tensor *b) {
    return tensor_compare_template(a, b, 0);
}

/* Confronto elementwise "maggiore di". */
tensor *tensor_gt(const tensor *a, const tensor *b) {
    return tensor_compare_template(a, b, 1);
}

/* Confronto elementwise di uguaglianza. */
tensor *tensor_eq(const tensor *a, const tensor *b) {
    return tensor_compare_template(a, b, 2);
}

/* Crea un tensore 1D che contiene le dimensioni del tensore a.
 * Se a è 1D con shape [N], ritorna un tensore 1D di 1 elemento [N].
 * Se a è 2D con shape [H W], ritorna un tensore 1D di 2 elementi [H W]. */
tensor *tensor_shape_tensor(const tensor *a) {
    if (!a) {
        return NULL;
    }

    int32_t ndim = a->ndim;
    if (ndim <= 0 || ndim > MAX_DIM) {
        return NULL;
    }

    /* Creo un tensore 1D che conterrà ndim elementi. */
    int32_t shape1d[1];
    shape1d[0] = ndim;

    tensor *res = tensor_create(shape1d, 1);
    if (!res) {
        return NULL;
    }

    /* Copio le dimensioni di a nei dati del tensore risultato. */
    for (int i = 0; i < ndim; ++i) {
        res->data[i] = (float)a->shape[i];
    }

    return res;
}

/* Crea un nuovo tensore che condivide i dati con a ma è visto come 1D.
 * Non ricopia il buffer; aumenta solo il refcount e adatta ndim/shape.
 */
tensor *tensor_ravel(tensor *a) {
    if (!a) {
        return NULL;
    }

    /* Creo una "view" che punta agli stessi dati. */
    tensor *res = malloc(sizeof(tensor));
    if (!res) {
        return NULL;
    }

    /* Condivide il buffer dati con a. */
    res->data = a->data;
    res->ndim = 1;
    res->shape[0] = (int32_t)a->size;  /* tutti gli elementi in un'unica dimensione. */
    res->shape[1] = 0;
    res->size = a->size;

    /* Condividiamo il refcount: incrementiamo quello di a, lo usiamo anche per res. */
    res->refcount = a->refcount + 1;
    a->refcount += 1;

    return res;
}

tensor *tensor_reshape(tensor *a, const tensor *s) {
        if (!a || !s) {
        fprintf(stderr, "DEBUG tensor_reshape: a o s è NULL\n");
        return NULL;
    }

    fprintf(stderr, "DEBUG tensor_reshape: s->ndim=%d, s->size=%zu\n",
            s->ndim, s->size);

    if (s->ndim != 1 || (s->size != 1 && s->size != 2)) {
        fprintf(stderr, "DEBUG tensor_reshape: s non è 1D con 1 o 2 elementi\n");
        return NULL;
    }

    int32_t new_shape[MAX_DIM];
    int32_t new_ndim = (int32_t)s->size;

    for (int32_t i = 0; i < new_ndim; ++i) {
        int val = (int)s->data[i];
        fprintf(stderr, "DEBUG tensor_reshape: s->data[%d]=%d\n", i, val);
        if (val <= 0) {
            fprintf(stderr, "DEBUG tensor_reshape: dimensione non positiva\n");
            return NULL;
        }
        new_shape[i] = (int32_t)val;
    }

    /* verifico che non cambi*/
    size_t new_size = tensor_num_elements(new_shape, new_ndim);
    fprintf(stderr, "DEBUG tensor_reshape: new_size=%zu, a->size=%zu\n",
            new_size, a->size);

    if (new_size != a->size) {
        fprintf(stderr, "DEBUG tensor_reshape: new_size != a->size\n");
        return NULL;
    }

    /* Creo una nuova "view" che condivide i dati con a ma con nuova shape. */
    tensor *res = malloc(sizeof(tensor));
    if (!res) {
        return NULL;
    }

    res->data = a->data;
    res->ndim = new_ndim;
    res->shape[0] = new_shape[0];
    res->shape[1] = (new_ndim == 2) ? new_shape[1] : 0;
    res->size = a->size;

    /* Condividiamo refcount con a. */
    res->refcount = a->refcount + 1;
    a->refcount += 1;

    return res;
}
