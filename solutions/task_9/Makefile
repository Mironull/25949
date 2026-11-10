CC = gcc
CFLAGS = -O2
SRC = process_manager.c
LONG_FILE = long_file.txt
LINES = 2000

# One source, several programs. They differ only in how the parent waits
# for the child (see the comment at the top of process_manager.c).
VARIANTS = process_nowait process_waitpid process_wait process_waitid \
           process_wait_early process_zombie

all: $(VARIANTS) $(LONG_FILE)

process_nowait: $(SRC)
	$(CC) $(CFLAGS) -DWAIT_MODE=MODE_NOWAIT -o $@ $(SRC)

process_waitpid: $(SRC)
	$(CC) $(CFLAGS) -DWAIT_MODE=MODE_WAITPID -o $@ $(SRC)

process_wait: $(SRC)
	$(CC) $(CFLAGS) -DWAIT_MODE=MODE_WAIT -o $@ $(SRC)

process_waitid: $(SRC)
	$(CC) $(CFLAGS) -DWAIT_MODE=MODE_WAITID -o $@ $(SRC)

process_wait_early: $(SRC)
	$(CC) $(CFLAGS) -DWAIT_MODE=MODE_WAIT_EARLY -o $@ $(SRC)

process_zombie: $(SRC)
	$(CC) $(CFLAGS) -DWAIT_MODE=MODE_ZOMBIE -o $@ $(SRC)

# Long file for cat: the bigger it is, the more visible the interleaving
# of parent and child output in the no-wait variant.
$(LONG_FILE):
	awk 'BEGIN { for (i = 1; i <= $(LINES); i++) print "Line number " i " of the long file" }' > $(LONG_FILE)

# Runs every variant and hides the lines of cat, keeping their numbers:
# the number in front of a parent line shows where it appeared in the output.
demo: all
	@for v in $(VARIANTS); do \
		echo "===== $$v"; \
		./$$v $(LONG_FILE) | grep -nv '^Line number'; \
		sleep 1; \
	done

clean:
	rm -f $(VARIANTS) $(LONG_FILE)

.PHONY: all demo clean
