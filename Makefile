CC = g++

CFLAGS  = -c -Wall
SOURCES = main.cpp httpserver.cpp 
OBJECTS = $(SOURCES:.cpp=.o)
LDFLAGS = -lpthread
TARGET = main

all: $(SOURCES) $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS) $(TARGET)
