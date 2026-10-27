#include <stdio.h>        // printf, fprintf, scanf
#include <stdlib.h>       // malloc, realloc, free, exit
#include <unistd.h>       // read, lseek, close
#include <fcntl.h>        // open, O_RDONLY

typedef struct {
    long offset;   // Позиция начала строки в файле (байт от начала)
    int length;    // Длина строки (количество символов)
} LineInfo;

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
        printf("offset: %ld, length: %d, count of bytes: %d\n", table[i-1].offset, table[i-1].length, abs(table[i-1].length - table[i-1].offset));
    }
    int num_line;
    while (1){
        printf("Введите номер строки от 1 до %d (0 - выход): ", num_lines);
        if (scanf("%d", &num_line) != 1) break;
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
