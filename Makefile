CC = g++

CFLAGS  = -c -Wall
SOURCES = main.cpp httpserver.cpp tcpstream.cpp http.cpp ./vendor/picohttpparser.cpp
INCLUDES = -I ./vendor
OBJECTS = $(SOURCES:.cpp=.o)
LDFLAGS = -lpthread
TARGET = main

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(INCLUDES) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(TARGET)
