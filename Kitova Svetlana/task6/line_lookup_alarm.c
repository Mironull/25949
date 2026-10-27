#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

const char *filename;

void write_all(int fd, const char *buffer, size_t length) {
    while (length > 0) {
        ssize_t written = write(fd, buffer, length);
        if (written <= 0) {
            return;
        }
        buffer += written;
        length -= (size_t)written;
    }
}

void alarm_handler(int signal_number) {
    (void)signal_number;
    const char message[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write_all(STDOUT_FILENO, message, sizeof(message) - 1);
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        _exit(EXIT_FAILURE);
    }
    char buffer[1024];
    ssize_t bytes_read;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write_all(STDOUT_FILENO, buffer, (size_t)bytes_read);
    }

    close(fd);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s filename\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }

    LineInfo *lines = NULL;
    int num_lines = 0;
    int capacity = 0;
    long line_start = 0;
    int length = 0;
    char ch;

    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            if (num_lines == capacity) {
                int new_capacity = capacity == 0 ? 8 : capacity * 2;
                LineInfo *temp = realloc(
                    lines, new_capacity * sizeof(LineInfo)
                );

                if (temp == NULL) {
                    perror("realloc");
                    free(lines);
                    close(fd);
                    return EXIT_FAILURE;
                }

                lines = temp;
                capacity = new_capacity;
            }

            lines[num_lines].offset = line_start;
            lines[num_lines].length = length;
            num_lines++;

            line_start = lseek(fd, 0L, SEEK_CUR);
            length = 0;
        } else {
            length++;
        }
    }

    if (length > 0) {
        if (num_lines == capacity) {
            int new_capacity = capacity == 0 ? 8 : capacity * 2;
            LineInfo *temp = realloc(
                lines, new_capacity * sizeof(LineInfo)
            );

            if (temp == NULL) {
                perror("realloc");
                free(lines);
                close(fd);
                return EXIT_FAILURE;
            }

            lines = temp;
            capacity = new_capacity;
        }

        lines[num_lines].offset = line_start;
        lines[num_lines].length = length;
        num_lines++;
    }

    printf("--- Line Table ---\n");

    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, lines[i].offset, lines[i].length);
    }

    struct sigaction action;
    action.sa_handler = alarm_handler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    if (sigaction(SIGALRM, &action, NULL) == -1) {
        perror("sigaction");
        free(lines);
        close(fd);
        return EXIT_FAILURE;
    }

    int number;

    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);
        alarm(5);
        int result = scanf("%d", &number);
        alarm(0);
        if (result != 1) {
            if (feof(stdin)) {
                break;
            }
            int c;
            while ((c = getchar()) != '\n' && c != EOF) {
            }
            if (c == EOF) {
                break;
            }
            printf("Please enter a number.\n");
            continue;
        }

        if (number == 0) {
            break;
        }

        if (number < 1 || number > num_lines) {
            printf("Invalid line number\n");
            continue;
        }

        int index = number - 1;
        int len = lines[index].length;

        if (lseek(fd, lines[index].offset, SEEK_SET) == -1) {
            perror("lseek");
            break;
        }

        char *buffer = malloc((size_t)len + 1);

        if (buffer == NULL) {
            perror("malloc");
            break;
        }

        ssize_t bytes_read = read(fd, buffer, (size_t)len);

        if (bytes_read == len) {
            buffer[len] = '\0';
            printf("%s\n", buffer);
        } else {
            perror("read");
        }

        free(buffer);
    }

    free(lines);
    close(fd);

    return EXIT_SUCCESS;
}

