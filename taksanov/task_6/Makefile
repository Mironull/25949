CC = gcc
CFLAGS = -O2 -std=gnu99 -Wall
TARGET = line_lookup_timeout
SRC = line_lookup_timeout.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)

.PHONY: all clean
