CF = -std=c++11 #-Wall -g

all: client.o ics.o
	g++ client.o ics.o -o client.out

client.o: client.cpp ics.h
	g++ -c $(CF) client.cpp -o client.o

ics.o: ics.cpp ics.h
	g++ -c $(CF) ics.cpp -o ics.o 

clean:
	rm *.out *.o

run:
	./client.out

reset: clean all