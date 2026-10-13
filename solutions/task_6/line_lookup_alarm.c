/* Задание 6. Таблица поиска строк с таймером: 5 секунд на ввод номера, иначе печать всего файла. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

typedef struct {
    long offset;    /* начало строки в файле */
    int length;     /* длина строки вместе с '\n' */
} LineInfo;

static int fd;      /* глобальный, чтобы обработчик сигнала мог читать файл */

static void alarm_handler(int sig)
{
    static const char msg[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    char buf[512];
    int n;

    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    lseek(fd, 0L, SEEK_SET);
    while ((n = read(fd, buf, sizeof(buf))) > 0)
        write(STDOUT_FILENO, buf, n);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int num_lines = 0, capacity = 16, n, i;
    long line_start = 0, pos;
    char c, *buf;
    LineInfo *lines;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror(argv[1]);
        exit(EXIT_FAILURE);
    }
    if ((lines = malloc(capacity * sizeof(LineInfo))) == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    /* Построение таблицы: позиция после каждого '\n' — начало следующей строки. */
    while (read(fd, &c, 1) == 1) {
        if (c != '\n')
            continue;
        pos = lseek(fd, 0L, SEEK_CUR);
        if (num_lines == capacity) {
            capacity *= 2;
            if ((lines = realloc(lines, capacity * sizeof(LineInfo))) == NULL) {
                perror("realloc");
                exit(EXIT_FAILURE);
            }
        }
        lines[num_lines].offset = line_start;
        lines[num_lines].length = pos - line_start;
        num_lines++;
        line_start = pos;
    }
    /* Последняя строка без '\n'. */
    pos = lseek(fd, 0L, SEEK_CUR);
    if (pos > line_start) {
        if (num_lines == capacity &&
            (lines = realloc(lines, (capacity + 1) * sizeof(LineInfo))) == NULL) {
            perror("realloc");
            exit(EXIT_FAILURE);
        }
        lines[num_lines].offset = line_start;
        lines[num_lines].length = pos - line_start;
        num_lines++;
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

        if ((buf = malloc(lines[i - 1].length + 1)) == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        lseek(fd, lines[i - 1].offset, SEEK_SET);
        n = read(fd, buf, lines[i - 1].length);
        if (n == -1) {
            perror("read");
            n = 0;
        }
        buf[n] = '\0';
        printf("%s", buf);
        free(buf);
    }

    close(fd);
    free(lines);
    return 0;
}
