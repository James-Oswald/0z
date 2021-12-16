#VulkanSDK=D:\Software\VulkanSDK\Lib
#https://gist.github.com/reecer/11065346
#https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html



CC= g++
CXXFLAGS= -g -O3 -std=c++2a
LDFLAGS=-Llib -lglfw3 -lgdi32
#Bug that forces you to link against Vulkan SDKs vulkan-1 dll rather than msys's
#https://github.com/glfw/glfw/issues/1900
#https://github.com/msys2/MINGW-packages/issues/5812
ifeq ($(OS),Windows_NT)
	LDFLAGS += -lvulkan-1
else
	LDFLAGS += -lvulkan
endif
INCLUDES=-Iinclude
SOURCES=$(wildcard *.cpp)
OBJECTS=$(SOURCES:%.cpp=./bin/%.o)

all : $(OBJECTS)
	$(CC) $(CXXFLAGS) -o ./bin/main.exe $? $(LDFLAGS)

./bin/%.o : %.cpp ./include/vkfw.hpp.gch
	$(CC) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) 

./include/vkfw.hpp.gch : ./include/vkfw.hpp
	$(CC) $(CXXFLAGS) $<