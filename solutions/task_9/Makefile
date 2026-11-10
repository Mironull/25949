CC = gcc
CFLAGS = -O2
TARGET = process_manager
SRC = process_manager.c
LONG_FILE = long_file.txt
LINES = 2000

all: $(TARGET) $(LONG_FILE)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Длинный файл для cat: чем он больше, тем заметнее перемешивание вывода
# родителя и ребёнка в режиме без ожидания.
$(LONG_FILE):
	awk 'BEGIN { for (i = 1; i <= $(LINES); i++) print "Line number " i " of the long file" }' > $(LONG_FILE)

# Первая часть задания: родитель не ждёт ребёнка.
nowait: all
	./$(TARGET) -n $(LONG_FILE)

# Вторая часть задания: родитель ждёт ребёнка через waitpid.
wait: all
	./$(TARGET) $(LONG_FILE)

clean:
	rm -f $(TARGET) $(LONG_FILE)

.PHONY: all nowait wait clean
