CC = gcc
CFLAGS = -O2
SRC = time_setuid.c

# Two copies of the same program with different permissions:
#   time_plain  (755)  - normal run, effective uid = uid of the caller;
#   time_suid   (4755) - setuid bit, effective uid = uid of the file owner.
PLAIN = time_plain
SUID = time_suid

all: $(PLAIN) $(SUID)
	ls -l $(PLAIN) $(SUID)

$(PLAIN): $(SRC)
	$(CC) $(CFLAGS) -o $(PLAIN) $(SRC)
	chmod 755 $(PLAIN)

$(SUID): $(SRC)
	$(CC) $(CFLAGS) -o $(SUID) $(SRC)
	chmod 4755 $(SUID)

clean:
	rm -f $(PLAIN) $(SUID)

.PHONY: all clean
