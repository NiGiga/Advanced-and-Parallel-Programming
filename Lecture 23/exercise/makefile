CC = gcc
CFLAGS = -O3 -Wall -std=c17 -pedantic -fopenmp -D_POSIX_C_SOURCE=200809L

all: main

main: main.o netpbm.o image_transform.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: netpbm.h image_transform.h
netpbm.o: netpbm.h
image_transform.o: image_transform.h netpbm.h

%.o: %.c
	$(CC) $(CFLAGS) -c $<

.PHONY: all clean

clean:
	rm -f *.o
	rm -f main
	rm -f kitty-transformed-*.pgm
