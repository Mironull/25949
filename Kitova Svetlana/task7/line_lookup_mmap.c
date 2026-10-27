#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    off_t offset;
    int length;
} LineInfo;

char *global_map;
off_t global_size;

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

    const char message[] =
        "\n[TIMEOUT] Time is up! Printing full file content...\n";

    write_all(STDOUT_FILENO, message, sizeof(message) - 1);

    if (global_size > 0) {
        write_all(STDOUT_FILENO, global_map, (size_t)global_size);
    }

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

    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    global_size = st.st_size;

    if (global_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return EXIT_SUCCESS;
    }

    global_map = mmap(
        NULL,
        (size_t)global_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0
    );

    if (global_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    close(fd);

    LineInfo *lines = NULL;
    int num_lines = 0;
    int capacity = 0;
    off_t line_start = 0;
    int length = 0;

    for (off_t i = 0; i < global_size; i++) {
        if (global_map[i] == '\n') {
            if (num_lines == capacity) {
                int new_capacity = capacity == 0 ? 8 : capacity * 2;

                LineInfo *temp = realloc(
                    lines,
                    new_capacity * sizeof(LineInfo)
                );

                if (temp == NULL) {
                    perror("realloc");
                    free(lines);
                    munmap(global_map, (size_t)global_size);
                    return EXIT_FAILURE;
                }

                lines = temp;
                capacity = new_capacity;
            }

            lines[num_lines].offset = line_start;
            lines[num_lines].length = length;
            num_lines++;

            line_start = i + 1;
            length = 0;
        } else {
            length++;
        }
    }

    if (length > 0) {
        if (num_lines == capacity) {
            int new_capacity = capacity == 0 ? 8 : capacity * 2;

            LineInfo *temp = realloc(
                lines,
                new_capacity * sizeof(LineInfo)
            );

            if (temp == NULL) {
                perror("realloc");
                free(lines);
                munmap(global_map, (size_t)global_size);
                return EXIT_FAILURE;
            }

            lines = temp;
            capacity = new_capacity;
        }

        lines[num_lines].offset = line_start;
        lines[num_lines].length = length;
        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");

    for (int i = 0; i < num_lines; i++) {
        printf(
            "Line %d: Offset = %ld, Length = %d\n",
            i + 1,
            (long)lines[i].offset,
            lines[i].length
        );
    }

    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

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

        fwrite(
            global_map + lines[index].offset,
            1,
            (size_t)lines[index].length,
            stdout
        );

        printf("\n");
    }

    free(lines);

    munmap(global_map, (size_t)global_size);

    return EXIT_SUCCESS;
}