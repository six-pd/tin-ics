all: test.o ics.o
	g++ test.o ics.o -o exe -lpthread

test.o: test.cpp
	g++ -c test.cpp -o test.o

ics.o: ../client/src/ics.cpp ../client/src/ics.h
	g++ -c ../client/src/ics.cpp -o ics.o
