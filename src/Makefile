CC=gcc
CFLAGS=-I. -std=c11
DEPS =
OBJ = a2unix.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

a2unix: $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS)
