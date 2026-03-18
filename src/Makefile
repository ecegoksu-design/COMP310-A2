CC=gcc
framesize ?= 18
varmemsize ?= 10
CFLAGS = -pthread -DFRAME_STORE_SIZE=$(framesize) -DVAR_STORE_SIZE=$(varmemsize)
FMT=indent

mysh: shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -c shell.c interpreter.c shellmemory.c
	$(CC) $(CFLAGS) -o mysh shell.o interpreter.o shellmemory.o

all: mysh
style: shell.c shell.h interpreter.c interpreter.h shellmemory.c shellmemory.h
	$(FMT) $?

test: test_all.sh
	./test_all.sh

clean:
	$(RM) mysh; $(RM) *.o; $(RM) *~

