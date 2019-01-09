CC = g++ -std=c++11

CFLAGS  = -c -Wall
SOURCES_DIR = ./src
SOURCES = $(wildcard *.cpp) ./vendor/picohttpparser.cpp
INCLUDES = -I ./vendor
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
