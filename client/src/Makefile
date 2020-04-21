CF = #-Wall -g

all: client.o
	g++ client.o -o client.out

client.o: client.cpp
	g++ -c $(CF) client.cpp -o client.o

clean:
	rm *.out *.o
