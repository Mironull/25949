CC = gcc
CFLAGS = -O2 -std=gnu99 -Wall
TARGET = line_lookup
SRC = line_lookup.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
