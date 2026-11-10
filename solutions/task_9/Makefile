CC = gcc
CFLAGS = -O2
TARGET = process_manager
SRC = process_manager.c
LONG_FILE = long_file.txt
LINES = 2000
MODES = nowait waitpid wait waitid early zombie

all: $(TARGET) $(LONG_FILE)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Long file for cat: the bigger it is, the more visible the interleaving
# of parent and child output in the nowait mode.
$(LONG_FILE):
	awk 'BEGIN { for (i = 1; i <= $(LINES); i++) print "Line number " i " of the long file" }' > $(LONG_FILE)

# Runs every mode and hides the lines of cat, keeping their numbers:
# the number in front of a parent line shows where it appeared in the output.
demo: all
	@for m in $(MODES); do \
		echo "===== $$m"; \
		./$(TARGET) $$m $(LONG_FILE) | grep -nv '^Line number'; \
		sleep 1; \
	done

clean:
	rm -f $(TARGET) $(LONG_FILE)

.PHONY: all demo clean
