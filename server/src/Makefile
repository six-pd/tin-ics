CF = #-Wall -g
LF = -lpthread

all: server.o
	gcc server.o -o server.out $(LF) 

server.o: server.c
	gcc -c $(CF) server.c -o server.o

clean:
	rm *.out *.o

run:
	./server.out
