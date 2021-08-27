
all : main.o
	g++ -O3 -o ./bin/main.exe ./bin/main.o

main.o : main.cpp
	g++ -c -O3 -o ./bin/main.o main.cpp