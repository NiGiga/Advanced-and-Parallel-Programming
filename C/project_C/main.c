/* Nicola Gigante - SM3201239
 * main.c
 * Programma principale: legge un file sorgente TensorForth e ne esegue i token.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "interpreter.h"
#include "stack.h"

#define MAX_TOKEN_LEN 1024

/* Stampa un messaggio di errore in base allo status restituito dall'interprete. */
static void print_error(interpreter_status st, const char *tok) {
    fprintf(stderr, "Errore durante l'esecuzione del token \"%s\": ",
            tok ? tok : "(null)");

    switch (st) {
        case INTERPRETER_STACK_ERROR:
            fprintf(stderr, "errore di stack (underflow o numero di operandi insufficiente).\n");
            break;
        case INTERPRETER_PARSE_ERROR:
            fprintf(stderr, "errore di parsing / token non valido.\n");
            break;
        case INTERPRETER_TYPE_ERROR:
            fprintf(stderr, "errore di tipo (operandi incompatibili per l'operazione richiesta).\n");
            break;
        case INTERPRETER_MEMORY_ERROR:
            fprintf(stderr, "errore di memoria (allocazione fallita).\n");
            break;
        default:
            fprintf(stderr, "errore sconosciuto.\n");
            break;
    }
}

/* Legge il prossimo token dal file sorgente in token_buf.
 * I token sono separati da spazi bianchi (spazi, tab, newline).
 *
 * Ritorna:
 *   0  se viene raggiunto EOF senza leggere alcun token,
 *   1  se un token è stato letto con successo.
 */
static int read_next_token(FILE *f, char *buf, size_t buf_size) {
    int c;

    /* Salta gli spazi bianchi iniziali (finché non trova un carattere non-space o EOF). */
    do {
        c = fgetc(f);
        if (c == EOF) {
            return 0;  /* nessun altro token da leggere */
        }
    } while (isspace(c));

    size_t i = 0;

    /* Caso 1: stringa tra doppi apici (es. "file.pgm"). */
    if (c == '"') {
        buf[i++] = (char)c;
        while (i + 1 < buf_size) {
            c = fgetc(f);
            if (c == EOF) {
                break;  /* stringa non chiusa: sarà rilevato come errore di parsing */
            }
            buf[i++] = (char)c;
            if (c == '"') {
                break;  /* fine stringa */
            }
        }
        buf[i] = '\0';
        return 1;
    }

    /* Caso 2: tensore 1D tra parentesi quadre (es. [1 2 3]). */
    if (c == '[') {
        buf[i++] = (char)c;
        while (i + 1 < buf_size) {
            c = fgetc(f);
            if (c == EOF) {
                break;  /* parentesi non chiusa: errore di parsing a valle */
            }
            buf[i++] = (char)c;
            if (c == ']') {
                break;  /* fine tensore */
            }
        }
        buf[i] = '\0';
        return 1;
    }

    /* Caso 3: token semplice (operatore, numero, ecc.), fino al prossimo spazio bianco o EOF. */
    buf[i++] = (char)c;
    while (i + 1 < buf_size) {
        c = fgetc(f);
        if (c == EOF || isspace(c)) {
            break;
        }
        buf[i++] = (char)c;
    }
    buf[i] = '\0';
    return 1;
}

int main(int argc, char **argv) {
    /* Controllo degli argomenti a riga di comando. */
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_file_sorgente>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    /* Apertura del file sorgente. */
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Impossibile aprire il file sorgente");
        return 1;
    }

    /* Creazione dello stack utilizzato dall'interprete. */
    stack *st = stack_create(16);
    if (!st) {
        fprintf(stderr, "Errore: impossibile creare lo stack.\n");
        fclose(f);
        return 1;
    }

    char token_buf[MAX_TOKEN_LEN];

    /* Lettura ed esecuzione dei token uno alla volta. */
    while (read_next_token(f, token_buf, sizeof(token_buf))) {
        interpreter_status st_code = execute_token(st, token_buf);
        if (st_code != INTERPRETER_OK) {
            /* In caso di errore, stampa messaggio, libera le risorse ed esce con codice != 0. */
            print_error(st_code, token_buf);
            stack_destroy(st);
            fclose(f);
            return 1;
        }
    }

    /* Chiusura file e rilascio finale delle risorse. */
    fclose(f);
    stack_destroy(st);

    /* Esecuzione completata senza errori. */
    return 0;
}
