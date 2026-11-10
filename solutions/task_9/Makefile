CC = gcc
CFLAGS = -O2
TARGET = process_manager
SRC = process_manager.c
LONG_FILE = long_file.txt
LINES = 2000

all: $(TARGET) $(LONG_FILE)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Long file for cat: the bigger it is, the more visible the interleaving
# of parent and child output in no-wait mode.
$(LONG_FILE):
	awk 'BEGIN { for (i = 1; i <= $(LINES); i++) print "Line number " i " of the long file" }' > $(LONG_FILE)

# First part of the task: the parent does not wait for the child.
nowait: all
	./$(TARGET) -n $(LONG_FILE)

# Second part of the task: the parent waits for the child with waitpid.
wait: all
	./$(TARGET) $(LONG_FILE)

clean:
	rm -f $(TARGET) $(LONG_FILE)

.PHONY: all nowait wait clean
