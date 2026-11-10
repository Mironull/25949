CC = gcc
CFLAGS = -O2
SRC = time_setuid.c

# Два экземпляра одной программы с разными правами:
#   time_plain  (755)  — обычный запуск, effective uid = uid запустившего;
#   time_suid   (4755) — бит setuid, effective uid = uid владельца файла.
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
