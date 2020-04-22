CF = -std=c++11 #-Wall -g
LF = -lpthread

all: main.o Client.o
	g++ main.o Client.o -o server.out $(LF) 

Client.o: Client.cpp Client.h
	g++ -c $(CF) Client.cpp -o Client.o

main.o: main.cpp
	g++ -c $(CF) main.cpp -o main.o

clean:
	rm *.out *.o

run:
	./server.out
