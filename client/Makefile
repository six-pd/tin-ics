CF = #-Wall -g

all: client.o
	gcc client.o -o client.out

client.o: client.c
	gcc -c $(CF) client.c -o client.o

clean:
	rm *.out *.o