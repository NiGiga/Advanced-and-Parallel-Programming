# Report di implementazione TensorForth (parte C)

## 1. Struttura generale: interprete stack-based

**Richiesto**

- Implementare un interprete per un linguaggio stack-based che legge una sequenza di token da file e li esegue da sinistra a destra usando uno stack di valori (tensori e stringhe).
- Segnalare come errore: stack vuoto o con elementi insufficienti, tipi incompatibili, dimensioni incompatibili.

**Implementato**

- **File:** `main.c`, `interpreter.c`

- In `main.c`:
  - `int main(int argc, char **argv)`:
    - Controlla gli argomenti (`argc != 2` → messaggio di uso e `return 1`).
    - Apre il file sorgente con `fopen`.
    - Crea lo stack con `stack *st = stack_create(16);`.
    - Legge i token uno alla volta con `read_next_token(FILE *f, char *buf, size_t buf_size)`.
    - Per ogni token chiama `execute_token(st, token_buf)`.
    - In caso di errore stampa un messaggio con `print_error`, chiude file e stack e ritorna `1`.

- In `interpreter.c`:
  - `interpreter_status execute_token(stack *st, const char *tok)`:
    - Interpreta il token (operazioni di stack, aritmetiche, logiche, reshape, I/O, ecc.).
    - Usa `stack_pop`/`stack_push` per gestire lo stack.
    - Controlla tipi e shape e ritorna uno dei valori `INTERPRETER_OK`, `INTERPRETER_STACK_ERROR`, `INTERPRETER_PARSE_ERROR`, `INTERPRETER_TYPE_ERROR`, `INTERPRETER_MEMORY_ERROR`.

**Perché così**

- Separare la logica di lettura/errore (`main.c`) dalla semantica del linguaggio (`interpreter.c`) mantiene il codice modulare e testabile.
- L’uso di un enum per lo stato di esecuzione consente una gestione uniforme degli errori.

---

## 2. Rappresentazione dei valori e dello stack

**Richiesto**

- Uno stack di valori eterogenei: tensori float 1D/2D e stringhe (per i filename).
- Le operazioni `d`, `s`, `o`, `D` devono manipolare lo stack senza copiare i tensori, ma aggiornando il refcount.

**Implementato**

- **File:** `stack.c`, `stack_ops.c`, `stack.h`

- In `stack.c`:
  - Struttura `stack` con:
    - `value *data;`
    - `int top;`
    - `int capacity;`.
  - `stack *stack_create(int initial_capacity)`:
    - Alloca la struttura e il buffer di `value`, inizializzando `top = 0` e una capacità iniziale (default 8).
  - `void stack_destroy(stack *st)`:
    - Scorre tutti gli elementi ancora nello stack.
    - Se `VALUE_TENSOR` → `tensor_dec_ref`.
    - Se `VALUE_STRING` → `free`.
  - `int stack_push(stack *st, value v)`:
    - Se necessario chiama `stack_grow` (realloc) per raddoppiare la capacità.
    - Inserisce `v` in posizione `top` e incrementa `top`.
  - `int stack_pop(stack *st, value *out)`:
    - Decrementa `top` e restituisce il `value` in `*out` senza toccare refcount/free (responsabilità del chiamante).

- In `stack_ops.c`:
  - `int op_dup(stack *st)`:
    - Legge il top; se è un tensore, chiama `tensor_inc_ref`.
    - `stack_push` di una copia del `value`.
    - Se il push fallisce, annulla l’incremento con `tensor_dec_ref`.
  - `int op_swap(stack *st)`:
    - Scambia gli ultimi due elementi dello stack con una semplice permuta.
  - `int op_over(stack *st)`:
    - Duplica il secondo elemento dall’alto, incrementando refcount se è un tensore, e lo pusha.
  - `int op_drop(stack *st)`:
    - `stack_pop` del valore in cima.
    - Se tensore → `tensor_dec_ref`; se stringa → `free`.

**Perché così**

- Lo stack rimane generico e ignora i dettagli di gestione memoria; il refcount dei tensori è centralizzato in `tensor.c`.
- Le operazioni `dup`/`over` non copiano i dati dei tensori, ma modificano solo il numero di riferimenti, come richiesto.

---

## 3. Tensore: struttura dati e primitive base

**Richiesto**

- Tensori 1D o 2D di `float`, con gestione corretta di:
  - shape (`int32_t shape[MAX_DIM]`, `ndim`),
  - numero di elementi,
  - refcount e deallocazione,
  - stampa per il debug.

**Implementato**

- **File:** `tensor.c`, `tensor.h`

- Struttura `tensor` (in `tensor.h`):
  - `float *data;`
  - `int32_t shape[MAX_DIM];`
  - `int32_t ndim;`
  - `size_t size;`
  - `int refcount;`
  - `void *mmap_base;`
  - `size_t mmap_size;`.

- Funzioni principali:
  - `size_t tensor_num_elements(const int32_t *shape, int32_t ndim)`:
    - Controlla che `ndim` sia tra 1 e `MAX_DIM` e che ogni `shape[i] > 0`.
    - Restituisce il prodotto delle dimensioni, 0 altrimenti.
  - `tensor *tensor_create(int32_t *shape, int32_t ndim)`:
    - Alloca la struct `tensor`.
    - Copia `shape` nei campi della struct, azzera le dimensioni non usate.
    - Calcola `size` tramite `tensor_num_elements`.
    - Alloca `data` come `malloc(size * sizeof(float))`.
    - Inizializza `refcount = 1`, `mmap_base = NULL`, `mmap_size = 0`.
  - `void tensor_inc_ref(tensor *t)` / `void tensor_dec_ref(tensor *t)`:
    - `tensor_inc_ref` incrementa il contatore se `t` non è `NULL`.
    - `tensor_dec_ref` decrementa il refcount e, se arriva a 0:
      - se `mmap_base != NULL`, chiama `munmap(mmap_base, mmap_size)`;
      - altrimenti libera `data` con `free` e infine `free(t)`.
  - `int tensor_same_shape(const tensor *a, const tensor *b)`:
    - Confronta `ndim` e ogni `shape[i]`.
  - `void tensor_print(const tensor *t)`:
    - Stampa nel formato `Tensor(shape=[...], data=[...])`, utilizzando solo le dimensioni effettive (`ndim`) e tutti i `size` valori.

**Perché così**

- La gestione unificata del refcount (incluso il caso `mmap`) permette a tutto il resto del codice di trattare i tensori in modo uniforme.
- Il formato di stampa è esattamente quello richiesto per l’operatore `p`, utile per il debug.
---

## 4. Operazioni aritmetiche: `+`, `-`, `*`

**Richiesto**

- Somma, differenza e prodotto elementwise:
  - `+ (b a -- a+b)`
  - `- (b a -- a-b)`
  - `* (b a -- a*b)`
- I tensori devono avere la stessa shape.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `tensor *tensor_add(const tensor *a, const tensor *b);`
  - `tensor *tensor_sub(const tensor *a, const tensor *b);`
  - `tensor *tensor_mul(const tensor *a, const tensor *b);`
  - Ogni funzione:
    - Verifica `a` e `b` non `NULL`.
    - Usa `tensor_same_shape(a, b)` per controllare shape compatibili.
    - Crea un nuovo tensore `res` con la stessa shape di `a`.
    - Itera su `a->size` e applica l’operazione elementwise (`res[i] = a[i] ± b[i]` o `a[i] * b[i]`).
    - In `tensor_add` è usato `#pragma omp parallel for` per parallellizzare il loop.

- In `interpreter.c`:
  - Blocco:
    ```c
    if ((tok == '+' || tok == '-' || tok == '*') && tok == '\0') { ... }
    ```
    - Pop di `va` (cima) e `vb` (secondo).
    - Controllo che siano `VALUE_TENSOR` e non `NULL`.
    - Scelta dell’operazione: `tensor_add(vb.as.t, va.as.t)`, ecc.
    - `tensor_dec_ref` su `va.as.t` e `vb.as.t`.
    - Push del valore risultato.

**Perché così**

- La semantica `(b a -- a op b)` è rispettata e implementata in modo chiaro.
- L’uso di funzioni dedicate in `tensor.c` rende facile testare le operazioni indipendentemente dallo stack.

---

## 5. Operazioni di confronto: `<`, `>`, `=`

**Richiesto**

- Confronti elementwise:
  - `< (b a -- a<b)`
  - `> (b a -- a>b)`
  - `= (b a -- a==b)`
- Output: tensore di 0.0/1.0 con stessa shape.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `static tensor *tensor_compare_template(const tensor *a, const tensor *b, int mode);`
    - Verifica `a`, `b` non `NULL`, e `tensor_same_shape(a, b)`.
    - Crea `res` con la stessa shape di `a`.
    - Per ogni elemento:
      - `mode == 0` → `av < bv`
      - `mode == 1` → `av > bv`
      - `mode == 2` → `av == bv`
    - Scrive `1.0f` se la condizione è vera, altrimenti `0.0f`.
  - Wrapper:
    - `tensor *tensor_lt(const tensor *a, const tensor *b);`
    - `tensor *tensor_gt(const tensor *a, const tensor *b);`
    - `tensor *tensor_eq(const tensor *a, const tensor *b);`

- In `interpreter.c`:
  - Blocco:
    ```c
    if ((tok == '<' || tok == '>' || tok == '=') && tok == '\0') { ... }
    ```
    - Pop di `va` e `vb`.
    - Controllo tipo `VALUE_TENSOR`.
    - Chiamata a `tensor_lt/gt/eq(vb.as.t, va.as.t)` seguendo `(b a -- a op b)`.
    - `tensor_dec_ref` su entrambi gli input.
    - Push del risultato.

**Perché così**

- La logica comune è centralizzata nel template, garantendo coerenza tra `<`, `>` ed `=`.
- La rappresentazione booleana 0.0/1.0 coincide con la specifica.

---

## 6. Operazioni logiche: `&`, `|`, `!`

**Richiesto**

- Operazioni logiche elementwise su tensori (interpretando 0.0 come falso, ≠0.0 come vero):
  - `& (b a -- a&b)` AND
  - `| (b a -- a|b)` OR
  - `! (a -- !a)` NOT
- Shape uguali.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `static tensor *tensor_logical_binary_template(const tensor *a, const tensor *b, int mode);`
    - Usa `tensor_same_shape`.
    - Converte i valori in booleani (`av = (a->data[i] != 0.0f)`).
    - `mode == 0` → `AND`, `mode == 1` → `OR`.
    - Scrive `1.0f` o `0.0f`.
  - `tensor *tensor_and(const tensor *a, const tensor *b);`
  - `tensor *tensor_or(const tensor *a, const tensor *b);`
  - `tensor *tensor_not(const tensor *a);`
    - Crea tensore con stessa shape, per ogni elemento invertendo 0.0 ↔ 1.0.

- In `interpreter.c`:
  - Blocco `&`/`|`:
    - Pop di due tensori, controlli tipo, `tensor_and`/`tensor_or`, dec-ref input, push risultato.
  - Blocco `!`:
    - Pop di un tensore, `tensor_not`, dec-ref input, push risultato.

**Perché così**

- Il comportamento è definito in `tensor.c` per mantenere coerenza tra tutte le operazioni logiche.
- L’interprete si limita a gestire lo stack e i tipi.

---

## 7. Operazioni speciali: ReLU, min, max (`R`, `m`, `M`)

**Richiesto**

- `R(a -- relu(a))` con `relu(x) = max(0,x)`.
- `m(ba--min(a,b))`, `M(ba--max(a,b))`: min/max elementwise con shape uguali.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `tensor *tensor_relu(const tensor *a)`:
    - Crea tensore `res` con la stessa shape.
    - Per ogni elemento: se `x > 0` → `x`, altrimenti `0`.
  - `tensor *tensor_min(const tensor *a, const tensor *b)`:
    - Usa `tensor_same_shape`.
    - `res[i] = (av < bv) ? av : bv`.
  - `tensor *tensor_max(const tensor *a, const tensor *b)`:
    - Analogamente con `max`.

- In `interpreter.c`:
  - Case `'R'`: pop tensore, `tensor_relu`, `tensor_dec_ref` sull’input, push risultato.
  - Case `'m'`/`'M'`: schema identico a `+ - *`, ma usando `tensor_min`/`tensor_max`.

**Perché così**

- Le operazioni elementwise speciali sono uniformi alle altre (stessa gestione di shape e refcount).

---

## 8. Operazioni sulla forma: `#`, `_`, `r`

**Richiesto**

- `#(a -- #a)`: tensore 1D che contiene le dimensioni di `a`.
- `_(a -- a')`: ravel, vista 1D di `a` con stessa memoria.
- `r(a s -- a')`: reshape di `a` alle dimensioni in `s`, stesso numero di elementi, stessa disposizione in memoria.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `tensor *tensor_shape_tensor(const tensor *a)`:
    - Crea un tensore 1D di lunghezza `a->ndim`.
    - Scrive `shape[i]` in `data[i]` come `float`.
  - `tensor *tensor_ravel(tensor *a)`:
    - Alloca una nuova struct `tensor` che condivide il buffer `data` con `a`.
    - Imposta `ndim = 1`, `shape[0] = a->size`, `size = a->size`.
    - Aggiorna i refcount in modo da rappresentare un nuovo riferimento logico condiviso.
  - `tensor *tensor_reshape(tensor *a, const tensor *s)`:
    - Verifica che `s` sia 1D con 1 o 2 elementi positivi.
    - Calcola `new_shape` e `new_size` tramite `tensor_num_elements`.
    - Se `new_size != a->size`, ritorna `NULL`.
    - Crea una struct `tensor` view che condivide `data` con `a`, ma con `ndim`/`shape` aggiornati.

- In `interpreter.c`:
  - Case `'#'`:
    - Pop di `a`, controlla tipo, chiama `tensor_shape_tensor(a.as.t)`, `tensor_dec_ref(a.as.t)`, push shape tensor.
  - Case `'_'`:
    - Pop di `a`, controlla tipo, chiama `tensor_ravel(a.as.t)`, dec-ref, push vista 1D.
  - Case `'r'`:
    - Pop di `vs` (shape) e `va` (tensore).
    - Controllo che entrambi siano tensori.
    - `tensor_reshape(va.as.t, vs.as.t)`, `tensor_dec_ref` su entrambi.
    - Push del tensore reshaped.

**Perché così**

- L’uso di view (nessuna copia di `data`) rispetta il vincolo che reshape/ravel non cambiano la disposizione dei valori in memoria.

---

## 9. Operazioni di generazione random: `?`

**Richiesto**

- `?(s -- a)`: genera un tensore con shape come in `s` (1D di 1 o 2 elementi) con valori random uniformi in `[0,1]`.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `tensor *tensor_random_from_shape(const tensor *s)`:
    - Verifica che `s` sia 1D, `s->size` sia 1 o 2 e che i valori siano >0.
    - Crea un tensore con shape derivata da `s`.
    - Riempie `data[i]` con `rand()/RAND_MAX`.

- In `interpreter.c`:
  - Case `'?'`:
    - Pop di `vs` (shape).
    - Controllo tipo tensore.
    - Chiamata a `tensor_random_from_shape(vs.as.t)`.
    - Dec-ref di `vs.as.t`.
    - Push del nuovo tensore.

**Perché così**

- È in linea con l’esempio di matrix multiplication del PDF, che usa `?` per generare matrici e vettori casuali.

---

## 10. Operazione di riduzione: `S`

**Richiesto**

- `S(a -- S(a))`: somma di tutti i valori in `a` in un tensore 1D di un elemento.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `tensor *tensor_sum_all(const tensor *a)`:
    - Crea tensore 1D con shape `[1]`.
    - Accumula la somma di tutti gli elementi in `sum`.
    - Assegna `res->data[0] = sum`.

- In `interpreter.c`:
  - Case `'S'`:
    - Pop di `a`, controllo tipo tensore.
    - `tensor_sum_all(va.as.t)`, `tensor_dec_ref(va.as.t)`, push risultato.

**Perché così**

- Si usa sempre un tensore per rappresentare anche gli “scalari”, come richiesto.

---

## 11. Operazione di filling: `f`

**Richiesto**

- `f(s v -- a)`: crea un tensore con shape definita da `s` e riempie con i valori di `v`, eventualmente ripetuti.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `tensor *tensor_fill_from_shape(const tensor *s, const tensor *v)`:
    - Verifica che `s` sia 1D e valida come shape (1-2 dimensioni positive).
    - Crea un tensore `a` con shape data da `s`.
    - Per ogni indice `i` di `a`, assegna un valore `v->data[i % v->size]`.

- In `interpreter.c`:
  - Case `'f'`:
    - Pop di `vv` (valori) e `vs` (shape).
    - Controllo che entrambi siano tensori.
    - Chiamata a `tensor_fill_from_shape(vs.as.t, vv.as.t)`.
    - `tensor_dec_ref` su `vs.as.t` e `vv.as.t`.
    - Push di `res`.

**Perché così**

- Permette pattern come `[5 5] [0.04] f` per costruire il kernel di blur 5x5 pieno di 0.04, come nell’esempio.

---

## 12. Operazione di utilità: `p`

**Richiesto**

- `p(a--)`: stampa il contenuto del tensore nel formato richiesto e consuma l’elemento.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - `void tensor_print(const tensor *t)`:
    - Stampa `Tensor(shape=[...], data=[...])` usando `printf`.

- In `interpreter.c`:
  - Case `'p'`:
    - Pop di `v`.
    - Se non è un tensore, libera eventuale stringa e ritorna `TYPE_ERROR`.
    - Altrimenti chiama `tensor_print(v.as.t)` e poi `tensor_dec_ref(v.as.t)`.

**Perché così**

- Fornisce un output leggibile per i test e il debug, come richiesto dalle specifiche.
---

## 13. Operazioni di manipolazione dello stack: `d`, `s`, `o`, `D`

**Richiesto**

- `d (a--aa)`, `s (ba--ab)`, `o (ba--bab)`, `D (a--)`.
- Dup/over devono aggiornare il refcount del tensore, non copiarlo.

**Implementato**

- **File:** `stack_ops.c`, `interpreter.c`

- In `stack_ops.c`:
  - Gestione refcount corretta come descritto nella sezione 2.

- In `interpreter.c`:
  - Nel blocco dei token a singolo carattere:
    - Case `'d'` → `op_dup(st)`.
    - Case `'s'` → `op_swap(st)`.
    - Case `'o'` → `op_over(st)`.
    - Case `'D'` → `op_drop(st)`.
  - In caso di ritorno 0 da queste funzioni, il `execute_token` restituisce `INTERPRETER_STACK_ERROR`.

**Perché così**

- Le operazioni dello stack sono isolate e facilmente testabili, mentre `interpreter.c` gestisce solo la mappatura simbolo → funzione.

---

## 14. I/O immagini PGM: `(` e `)`

**Richiesto**

- `((filename -- tensor)`: lettura di PGM binario (P5) come tensore 2D con valori in [0,1].
- `)(a filename --)`: scrittura di tensore 2D come PGM P5, clampando in [0,1] e mappando in [0,255].

**Implementato**

- **File:** `pgm_io.c`, `interpreter.c`

- In `pgm_io.c`:
  - `tensor *tensor_read_pgm(const char *filename)`:
    - Apre il file in binario.
    - Legge magic `P5`, width, height, maxval.
    - Crea tensore 2D `[height, width]`.
    - Legge `width*height` byte e li converte in float in `[0,1]` dividendo per `maxval`.
  - `int tensor_write_pgm(const tensor *img, const char *filename)`:
    - Verifica che `img` sia 2D.
    - Apre il file, scrive header `P5`, dimensioni e `maxval=255`.
    - Converte ogni valore in `[0,1]` a byte `[0,255]` con clamp e scaling.

- In `interpreter.c`:
  - Case `'('`:
    - Pop `vfile` (stringa), controllo tipo, `tensor_read_pgm(vfile.as.s)`, `free(vfile.as.s)`, push tensore.
  - Case `')'`:
    - Pop `vfile` e `va` (tensore).
    - Controllo tipi (stringa + tensore).
    - `tensor_write_pgm(va.as.t, vfile.as.s)`, `tensor_dec_ref(va.as.t)`, `free(vfile.as.s)`.

**Perché così**

- L’interprete non contiene logica specifica del formato PGM, delegata a `pgm_io.c`.
---

## 15. I/O binario con mmap: `{` e `}`

**Richiesto**

- `{(filename -- tensor)`: lettura da disco nel formato `on_disk_tensor` usando `mmap`.
- `}(a filename --)`: salvataggio nel formato `on_disk_tensor` con `data_offset` allineato a 64 byte.

**Implementato**

- **File:** `tensor_io.c`, `interpreter.c`

- In `tensor_io.c`:
  - `tensor *tensor_load_from_file_mmap(const char *filename)`:
    - `open` in sola lettura, `fstat` per dimensione.
    - Verifica che `st.st_size >= sizeof(on_disk_tensor)`.
    - `mmap` dell’intero file in memoria.
    - Interpreta l’inizio come `on_disk_tensor` (shape[], ndim, data_offset).
    - Verifica `ndim`, shape positive e `nelems` con `tensor_num_elements`.
    - Controlla che `data_offset + nelems*sizeof(float)` sia dentro il file.
    - Crea una `tensor` che punta direttamente ai dati mappati, impostando `mmap_base` e `mmap_size`.
  - `int tensor_save_to_file(const tensor *t, const char *filename)`:
    - Apre il file in scrittura binaria.
    - Costruisce un `on_disk_tensor header` con `ndim`, `shape[]`, `data_offset = 64`.
    - Scrive l’header.
    - Aggiunge padding di zeri fino a raggiungere `data_offset`.
    - Scrive tutti i `float` di `t->data`.

- In `interpreter.c`:
  - Case `'{'`:
    - Pop `vfile` (stringa), `tensor_load_from_file_mmap(vfile.as.s)`, `free(vfile.as.s)`, push tensore.
  - Case `'}'`:
    - Pop `vfile` e `va` (tensore).
    - `tensor_save_to_file(va.as.t, vfile.as.s)`.
    - `tensor_dec_ref(va.as.t)`, `free(vfile.as.s)`.

**Perché così**

- L’uso di `mmap` evita copie in RAM e rispetta il formato richiesto, con i dati allineati a 64 byte dal file.

---

## 16. Operazioni specifiche per tensori: `@`, `.`, `c`

**Richiesto**

- `@(b a -- a@b)`: prodotto di matrici 2D compatibili.
- `.(b a -- a.b)`: dot product per vettori 1D.
- `c(a k -- conv(a,k))`: convoluzione 2D con kernel 2D, stride 1, zero-padding.

**Implementato**

- **File:** `tensor.c`, `interpreter.c`

- In `tensor.c`:
  - Funzioni (non tutte riportate nei frammenti, ma implementate in `tensor-5.c`):
    - `tensor *tensor_matmul(const tensor *a, const tensor *b)`: controlla che `a` e `b` siano 2D e che dimensioni interne combacino, esegue prodotto di matrici.
    - `tensor *tensor_dot(const tensor *a, const tensor *b)`: richiede vettori 1D di stessa lunghezza e restituisce un tensore 1D di un elemento con il dot product.
    - `tensor *tensor_conv2d(const tensor *a, const tensor *k)`: applica una convoluzione 2D con padding zero e stride 1.

- In `interpreter.c`:
  - Blocco `@`:
    - Pop `va`, `vb`, controlli tipo tensore 2D.
    - `tensor_matmul(vb.as.t, va.as.t)`, dec-ref input, push risultato.
  - Blocco `.`:
    - Pop `va`, `vb`, controlli tipo tensore 1D.
    - `tensor_dot(vb.as.t, va.as.t)`, dec-ref input, push risultato.
  - Blocco `c`:
    - Pop `vk` (kernel) e `va` (immagine).
    - `tensor_conv2d(va.as.t, vk.as.t)`, dec-ref input, push risultato.

**Perché così**

- Si mantiene la semantica `(b a -- a op b)` per tutte le operazioni binarie.
- I controlli di shape sono incapsulati in `tensor.c`, mentre l’interprete gestisce solo tipologia di valore e refcount.

---

## 17. Gestione errori e uscita

**Richiesto**

- In caso di errore (stack, tipo, dimensioni, I/O, memoria):
  - segnalare l’errore con un messaggio;
  - uscire con un codice di errore diverso da 0;
  - non avere segmentation fault.

**Implementato**

- **File:** `main.c`, `interpreter.c`

- In `interpreter.c`:
  - Ogni case controlla:
    - esito di `stack_pop` (se fallisce → `INTERPRETER_STACK_ERROR`);
    - tipo del `value` (se non compatibile → `INTERPRETER_TYPE_ERROR`);
    - esito delle funzioni `tensor_*` (se restituiscono `NULL` → `INTERPRETER_TYPE_ERROR` o `INTERPRETER_MEMORY_ERROR`).
  - Rilascia sempre le risorse già poppate prima di restituire un errore.

- In `main.c`:
  - `print_error(interpreter_status st, const char *tok)`:
    - Traduce il codice di stato in un messaggio su `stderr` con il token coinvolto.
  - Nel loop principale:
    - se `execute_token` ritorna uno stato diverso da `INTERPRETER_OK`:
      - stampa il messaggio;
      - chiude file e distrugge lo stack;
      - ritorna `1`.

**Perché così**

- Tutti i punti critici (allocazioni, pop di stack, letture file, operazioni su tensori) sono verificati e reagiscono in modo controllato.
- In nessun caso si accede a puntatori `NULL` o a dati fuori dallo stack senza controlli, evitando segmentation fault.
