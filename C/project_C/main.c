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

/* Funzione di utilità: stampa un messaggio di errore in base allo status. */
static void print_error(interpreter_status st, const char *tok) {
    fprintf(stderr, "Errore durante l'esecuzione del token \"%s\": ", tok ? tok : "(null)");

    switch (st) {
        case INTERPRETER_STACK_ERROR:
            fprintf(stderr, "errore di stack (underflow o simile).\n");
            break;
        case INTERPRETER_PARSE_ERROR:
            fprintf(stderr, "errore di parsing / token non valido.\n");
            break;
        case INTERPRETER_TYPE_ERROR:
            fprintf(stderr, "errore di tipo (operandi incompatibili).\n");
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
 * - Se trova EOF, ritorna 0.
 * - Se legge un token valido, ritorna 1.
 */
static int read_next_token(FILE *f, char *buf, size_t buf_size) {
    int c;

    /* Salto spazi bianchi iniziali. */
    do {
        c = fgetc(f);
        if (c == EOF) {
            return 0;
        }
    } while (isspace(c));

    size_t i = 0;

    /* Caso 1: stringa tra doppi apici. */
    if (c == '"') {
        buf[i++] = (char)c;
        while (i + 1 < buf_size) {
            c = fgetc(f);
            if (c == EOF) {
                break;
            }
            buf[i++] = (char)c;
            if (c == '"') {
                break;  /* fine stringa */
            }
        }
        buf[i] = '\0';
        return 1;
    }

    /* Caso 2: tensore tra parentesi quadre. */
    if (c == '[') {
        buf[i++] = (char)c;
        while (i + 1 < buf_size) {
            c = fgetc(f);
            if (c == EOF) {
                break;
            }
            buf[i++] = (char)c;
            if (c == ']') {
                break;  /* fine tensore */
            }
        }
        buf[i] = '\0';
        return 1;
    }

    /* Caso 3: token semplice (operatore, nome comando, numero, ecc.). */
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
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <nome_file_sorgente>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    /* Apro il file sorgente. */
    FILE *f = fopen(filename, "r");
    if (!f) {
        perror("Impossibile aprire il file sorgente");
        return 2;
    }

    /* Creo lo stack usato dall'interprete. */
    stack *st = stack_create(16);
    if (!st) {
        fprintf(stderr, "Errore: impossibile creare lo stack.\n");
        fclose(f);
        return 3;
    }

    char token_buf[MAX_TOKEN_LEN];

    /* Leggo i token usando read_next_token.
     * Ogni token viene passato a execute_token.
     */
    while (read_next_token(f, token_buf, sizeof(token_buf))) {
      printf("TOKEN: '%s'\n", token_buf);  // DEBUG: stampa ogni token letto
      
      interpreter_status st_code = execute_token(st, token_buf);
      if (st_code != INTERPRETER_OK) {
        print_error(st_code, token_buf);
        stack_destroy(st);
        fclose(f);
        return 4;
      }
    }

    /* Chiusura file e rilascio risorse. */
    fclose(f);
    stack_destroy(st);

    return 0;
}
