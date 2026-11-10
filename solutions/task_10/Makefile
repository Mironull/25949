CC = gcc
CFLAGS = -O2
TARGET = run_command
SRC = run_command.c

all: $(TARGET) crash_program exit_code

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Тестовая программа, которая падает с SIGSEGV.
crash_program: crash_program.c
	$(CC) -O0 -o crash_program crash_program.c

# Тестовая программа, которая завершается с заданным кодом.
exit_code: exit_code.c
	$(CC) $(CFLAGS) -o exit_code exit_code.c

# Прогоняет все случаи из методички.
test: all
	@echo "== Тест 1: успешная команда"
	-./$(TARGET) ls -l /tmp
	@echo "== Тест 2: команда с ненулевым кодом"
	-./$(TARGET) grep nonexistent_string_123 /etc/passwd
	@echo "== Тест 3: команда не найдена"
	-./$(TARGET) fake_command_123
	@echo "== Тест 4: падение по сигналу"
	-./$(TARGET) ./crash_program
	@echo "== Тест 5: произвольный код завершения"
	-./$(TARGET) ./exit_code 42

clean:
	rm -f $(TARGET) crash_program exit_code core

.PHONY: all test clean
