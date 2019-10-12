CC = gcc
CCOPTS = -c -g -Wall -ggdb 
LINKOPTS = -g -pthread

all: test

test: test.o 

test.o: test.c
	$(CC) $(CCOPTS) -o $@ test.c

clean:
	rm -rf test test.exe *.o *~ *.err *.out