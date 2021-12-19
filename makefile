#VulkanSDK=D:\Software\VulkanSDK\Lib
#https://gist.github.com/reecer/11065346
#https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html



CC= g++
CXXFLAGS= -g -std=c++2a
LDFLAGS= -Llib -lglfw3 -lgdi32 -lboost_filesystem-mt
#Bug that forces you to link against Vulkan SDKs vulkan-1 dll rather than msys's
#https://github.com/glfw/glfw/issues/1900
#https://github.com/msys2/MINGW-packages/issues/5812
ifeq ($(OS),Windows_NT)
	LDFLAGS += -lvulkan-1
else
	LDFLAGS += -lvulkan
endif
INCLUDES=-Iinclude
SOURCES=$(wildcard ./src/*.cpp)
OBJECTS=$(SOURCES:./src/%.cpp=./bin/%.o)
LAYERSRCS=$(wildcard ./lib/VkLayer*)
LAYERDEST=$(LAYERSRCS:./lib/%=./bin/layers/%)

all : $(OBJECTS) $(LAYERDEST) ./bin/config.json
	$(CC) $(CXXFLAGS) -o ./bin/main.exe $(OBJECTS) $(LDFLAGS)

./bin/%.o : ./src/%.cpp ./include/master.hpp.gch ./src/Utils.hpp
	$(CC) $(CXXFLAGS) -c -o $@ $< $(INCLUDES)

./bin/layers/VkLayer% : ./lib/VkLayer%
	cp $< $@

./bin/config.json : ./src/config.json
	cp $< $@

./include/master.hpp.gch : ./include/master.hpp ./include/vkfw.hpp
	$(CC) $(CXXFLAGS) $< $(INCLUDES)

setup:
	mkdir -p ./bin
	mkdir -p ./bin/layers

clean:
	rm ./bin/*.o