#include <stdio.h>        // printf, fprintf, scanf
#include <stdlib.h>       // malloc, realloc, free, exit
#include <unistd.h>       // read, lseek, close
#include <fcntl.h>        // open, O_RDONLY
#include <signal.h>
#include <string.h>

static int g_fd = -1;
static const char *g_path = NULL;

typedef struct {
    long offset;   // Позиция начала строки в файле (байт от начала)
    int length;    // Длина строки (количество символов)
} LineInfo;

void alarm_handler(int signo)
{
    (void)signo;
    const char *msg = "[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, strlen(msg));

    if (g_fd >= 0) {
        lseek(g_fd, 0, SEEK_SET);
        char buf[4096];
        ssize_t n;
        while ((n = read(g_fd, buf, sizeof buf)) > 0) {
            char* p = buf;
            ssize_t left = n;
            while (left > 0) {
                ssize_t w = write(STDOUT_FILENO, p, left);
                if (w < 0) _exit(EXIT_FAILURE);
                p+= w;
                left -=w;
            }
        }
    }
    _exit(EXIT_SUCCESS);
}


int main(int argc, char* argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Не получилось с %s файлом\n", argv[0]);
        return 1;
    }
    int file = open(argv[1], O_RDONLY);

    if (file == -1)
    {
        perror("open");
        return 1;
    }

    g_path = argv[1];
    g_fd = file;
    signal(SIGALRM, alarm_handler);

    LineInfo *table = malloc(sizeof(LineInfo));
    if (table == NULL) {
        perror("malloc");
        close(file);
        return 1;
    }
    char c;
    int num_lines = 0;
    int capacity = 1;
    long cur_pos = 0;
    long line_start = 0;

    while (read(file, &c, 1) == 1)
    {
        if (c == '\n')
        {
            if (num_lines == capacity)
            {
                capacity*=2;
                LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
                if (tmp == NULL) {
                    perror("realloc");
                    free(table);
                    close(file);
                    return 1;
                }
                table = tmp;
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = cur_pos - line_start;
            num_lines++;
            line_start = cur_pos + 1;            
        }
        cur_pos++;
    }

    if (line_start < cur_pos)
    {
        if (num_lines == capacity)
        {
            capacity*=2;
            LineInfo *tmp = realloc(table, capacity * sizeof(LineInfo));
            if (tmp == NULL) {
                perror("realloc");
                free(table);
                close(file);
                return 1;
            }
            table = tmp;
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = cur_pos - line_start;
        num_lines++;
    }

    for (int i = 1; i < num_lines+1; i++)
    {
        printf("line %d: offset = %ld, length = %d\n", i, table[i-1].offset, table[i-1].length);
    }
    int num_line;
    while (1){
        printf("У вас есть 5 секунд. Введите номер строки от 1 до %d (0 - выход): ", num_lines);
        fflush(stdout);
        alarm(5);
        if (scanf("%d", &num_line) != 1) {
            alarm(0);
            int ch;
            while((ch = getchar()) != '\n' && ch != EOF) {}
            printf("Invalid input\n");
            continue;
        }
        alarm(0);
        if (num_line == 0) break;
        if (num_line < 1 || num_line > num_lines) {
            printf("Недопустимый номер\n");
            continue;
        }
        LineInfo li = table[num_line - 1];
        char *line = malloc(li.length + 1);
        if (line == NULL) {
            perror("malloc");
            break;
        }
        lseek(file, li.offset, SEEK_SET);
        read(file, line, li.length);
        line[li.length] = '\0';

        printf("%s\n", line);
        free(line);
    }    

    close(file);
    free(table);
    
    return 0;
}
