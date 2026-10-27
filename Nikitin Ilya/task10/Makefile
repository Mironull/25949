CC = gcc
CFLAGS = -O2
TARGET = run_command crash_program
SRC = run_command.c crash_program.c

all: $(TARGET)

%: %.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)

.PHONY: all clean
