#VulkanSDK=D:\Software\VulkanSDK\Lib
#https://gist.github.com/reecer/11065346
#https://www.gnu.org/software/make/manual/html_node/Automatic-Variables.html



CC= g++
CXXFLAGS= -g -std=c++2a -H
LOGGING= > ./logs/$(@F).log 2>&1
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
HEADERS=$(wildcard ./include/*.h)
COMPHEADERS=$(LAYERSRCS:./lib/%=./bin/layers/%)

all : $(OBJECTS) $(LAYERDEST) ./bin/config.json 
	$(CC) $(CXXFLAGS) -o ./bin/main.exe $(OBJECTS) $(LDFLAGS) $(LOGGING)

./bin/%.o : ./src/%.cpp ./src/%.hpp ./src/%.tpp $(COMPHEADERS)
	$(CC) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) $(LOGGING)

./bin/%.o : ./src/%.cpp ./src/%.hpp $(COMPHEADERS)
	$(CC) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) $(LOGGING)

./bin/%.o : ./src/%.cpp $(COMPHEADERS)
	$(CC) $(CXXFLAGS) -c -o $@ $< $(INCLUDES) $(LOGGING)

./include/%.gch : ./include/%.hpp
	$(CC) $(CXXFLAGS) $< $(INCLUDES)

./bin/layers/VkLayer% : ./lib/VkLayer%
	cp $< $@

./bin/config.json : ./src/config.json
	cp $< $@

setup:
	mkdir -p ./logs
	mkdir -p ./bin
	mkdir -p ./bin/layers

clean:
	rm -R ./bin/*
	mkdir -p ./bin/layers
