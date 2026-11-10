CC = gcc
CFLAGS = -O2
TARGET = run_command
SRC = run_command.c

all: $(TARGET) crash_program exit_code

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Test program that crashes with SIGSEGV.
crash_program: crash_program.c
	$(CC) -O0 -o crash_program crash_program.c

# Test program that exits with the given code.
exit_code: exit_code.c
	$(CC) $(CFLAGS) -o exit_code exit_code.c

# Runs all the checks from the task description.
test: all
	@echo "== Test 1: successful command"
	-./$(TARGET) ls -l /tmp
	@echo "== Test 2: command with a non-zero exit code"
	-./$(TARGET) grep nonexistent_string_123 /etc/passwd
	@echo "== Test 3: command not found"
	-./$(TARGET) fake_command_123
	@echo "== Test 4: killed by a signal"
	-./$(TARGET) ./crash_program
	@echo "== Test 5: arbitrary exit code"
	-./$(TARGET) ./exit_code 42

clean:
	rm -f $(TARGET) crash_program exit_code core

.PHONY: all test clean
