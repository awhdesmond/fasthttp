CC = g++ -std=c++11

CFLAGS  = -c -Wall
SOURCES_DIR = ./src
SOURCES = $(wildcard *.cpp) $(wildcard ./vendor/yaml-cpp/src/*.cpp) ./vendor/picohttpparser/picohttpparser.cpp
INCLUDES = -I ./vendor/picohttpparser -I ./vendor/yaml-cpp/
OBJECTS = $(SOURCES:.cpp=.o)
LDFLAGS = -pthread 
TARGET = main

all: $(SOURCES) $(TARGET) 

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@ $(LDFLAGS)

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(TARGET)
