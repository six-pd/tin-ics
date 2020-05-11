CF = -std=c++11 #-Wall -g
LF = -lpthread

all: main.o ClientHandling.o
	g++ main.o ClientHandling.o -o server.out $(LF) 

ClientHandling.o: ClientHandling.cpp ClientHandling.h
	g++ -c $(CF) ClientHandling.cpp -o ClientHandling.o

main.o: main.cpp
	g++ -c $(CF) main.cpp -o main.o

clean:
	rm *.out *.o

run:
	./server.out

reset: clean all