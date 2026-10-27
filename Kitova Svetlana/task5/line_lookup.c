#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct {
    long offset;
    int length;
} LineInfo;

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

    int number;

    while (1) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        if (scanf("%d", &number) != 1) {
            break;
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

