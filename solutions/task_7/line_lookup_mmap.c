/* Задание 7. Таблица поиска строк с таймером, файл отображается в память (mmap). */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct {
    long offset;    /* начало строки в файле */
    int length;     /* длина строки вместе с '\n' */
} LineInfo;

/* Глобальные, чтобы обработчик сигнала мог вывести файл. */
static char *map;
static size_t size;

static void alarm_handler(int sig)
{
    static const char msg[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";

    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    write(STDOUT_FILENO, map, size);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int fd, num_lines = 0, capacity = 16, n, i;
    long line_start = 0, pos;
    struct stat st;
    LineInfo *lines;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        exit(EXIT_FAILURE);
    }
    if (st.st_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }
    size = st.st_size;
    if ((map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0)) == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }
    close(fd);

    if ((lines = malloc(capacity * sizeof(LineInfo))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* Построение таблицы: позиция после каждого '\n' — начало следующей строки.
     * Последняя строка без '\n' обрабатывается на pos == size. */
    for (pos = 0; pos < (long)size; pos++) {
        if (map[pos] != '\n' && pos != (long)size - 1)
            continue;
        if (num_lines == capacity) {
            capacity *= 2;
            if ((lines = realloc(lines, capacity * sizeof(LineInfo))) == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }
        lines[num_lines].offset = line_start;
        lines[num_lines].length = pos + 1 - line_start;
        num_lines++;
        line_start = pos + 1;
    }

    printf("--- Debug: Line Table ---\n");
    for (i = 0; i < num_lines; i++)
        printf("Line %d: Offset = %ld, Length = %d\n",
               i + 1, lines[i].offset, lines[i].length);
    printf("-------------------------\n");

    signal(SIGALRM, alarm_handler);

    for (;;) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);
        n = scanf("%d", &i);
        alarm(0);
        if (n == EOF)
            break;
        if (n == 0) {
            while ((n = getchar()) != '\n' && n != EOF)
                ;
            printf("Not a number\n");
            continue;
        }
        if (i == 0)
            break;
        if (i < 1 || i > num_lines) {
            printf("No such line (1..%d)\n", num_lines);
            continue;
        }

        fwrite(map + lines[i - 1].offset, 1, lines[i - 1].length, stdout);
    }

    free(lines);
    munmap(map, size);
    return 0;
}
