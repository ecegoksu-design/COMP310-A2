CC=gcc
CFLAGS=
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

