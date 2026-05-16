# Nicola Gigante - SM3201239
# Makefile per compilare l'interprete TensorForth in C.

CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c11 -fopenmp
LDFLAGS = -fopenmp

# Lista dei file sorgente
SRCS = main.c tensor.c stack.c stack_ops.c interpreter.c
OBJS = $(SRCS:.c=.o)

# Target principale: produce l'eseguibile 'tensorforth'
tensorforth: $(OBJS)
	$(CC) $(CFLAGS) -o tensorforth $(OBJS) $(LDFLAGS)

# Regola generica per compilare i .c in .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Pulizia
clean:
	rm -f $(OBJS) tensorforth
