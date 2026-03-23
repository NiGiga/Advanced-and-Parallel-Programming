CC = gcc
CFLAGS = -O3 -Wall -pedantic -std=c17
LIB = lib
HEADERS = tree_node.h bst.h print_tree.h
OBJ = tree_node.o bst.o print_tree.o
LIB_NAME = libbst
TYPE = static

ifeq (${TYPE}, dynamic)
	EXT = so
	CFLAGS += -fPIC
else
	EXT = a
endif

TARGET_LIB = ${LIB}/${LIB_NAME}.${EXT}

all: ${TARGET_LIB} main

main: main.c ${TARGET_LIB}
	${CC} ${CFLAGS} main.c -L${LIB} -lbst -o main

${TARGET_LIB}: ${OBJ}
	mkdir -p ${LIB}
ifeq (${TYPE}, dynamic)
	# Command to make dynamic lib
	${CC} -shared -o ${TARGET_LIB} ${OBJ}
else
	# Command to make static lib
	ar rcs ${TARGET_LIB} ${OBJ}
endif

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -I.  -c $< -o $@

.PHONY: clean

clean:
	rm -f *.o main
	rm -rf ${LIB}
