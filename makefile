
#https://gist.github.com/reecer/11065346
#https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html

CC= g++
CXXFLAGS= -g -std=c++2a
LDFLAGS=-Llib -lvulkan -lglfw3 -lgdi32
INCLUDES=-Iinclude
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:%.cpp=./bin/%.o)

all : $(OBJECTS)
	$(CC) -o ./bin/main.exe $? $(LDFLAGS)

./bin/%.o : %.cpp ./include/vkfw.hpp.gch
	$(CC) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) 

./include/vkfw.hpp.gch : ./include/vkfw.hpp
	$(CC) $(CXXFLAGS) $<