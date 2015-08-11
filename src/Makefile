CC=gcc
CFLAGS=-I. -std=c11 -Wno-implicit-function-declaration -O3
DEPS =
OBJ = a2unix.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

a2unix: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~ core a2unix
