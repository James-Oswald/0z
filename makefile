
all : main.o
	g++ -O3 -std=c++2a -o ./bin/main.exe ./bin/main.o -Llib -lvulkan -lglfw3 -lgdi32

main.o : main.cpp
	g++ -c -O3 -std=c++2a -o ./bin/main.o main.cpp -Iinclude 