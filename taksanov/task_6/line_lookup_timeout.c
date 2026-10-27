#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/time.h>

typedef struct {
    off_t offset;
    int length;
} LineInfo;

// Функция печати всего файла целиком при истечении таймаута
void print_entire_file(int fd) {
    printf("\nTime is up (5 seconds expired)! Printing entire file:\n");
    printf("========================================\n");
    
    // Перематываем файл в самое начало
    lseek(fd, 0, SEEK_SET);

    char buf[1024];
    ssize_t bytes;
    while ((bytes = read(fd, buf, sizeof(buf))) > 0) {
        write(STDOUT_FILENO, buf, bytes);
    }
    printf("========================================\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return 1;
    }

    // 1. Построение таблицы смещений строк
    int capacity = 16;
    int num_lines = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (!table) {
        perror("malloc failed");
        close(fd);
        return 1;
    }

    off_t current_pos = 0;
    off_t line_start = 0;
    int line_len = 0;
    char ch;

    while (read(fd, &ch, 1) == 1) {
        current_pos++;
        line_len++;

        if (ch == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                LineInfo *new_table = realloc(table, capacity * sizeof(LineInfo));
                if (!new_table) {
                    perror("realloc failed");
                    free(table);
                    close(fd);
                    return 1;
                }
                table = new_table;
            }

            table[num_lines].offset = line_start;
            table[num_lines].length = line_len;
            num_lines++;

            line_start = current_pos;
            line_len = 0;
        }
    }

    if (line_len > 0) {
        if (num_lines >= capacity) {
            capacity += 1;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = line_len;
        num_lines++;
    }

    // 2. Отладочный вывод таблицы
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", 
               i + 1, (long)table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // 3. Цикл запроса с 5-секундным таймером через select()
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout); // Принудительно выводим текст приглашения

        // Настраиваем набор дескрипторов для select (ждем только ввод со stdin)
        fd_set readfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);

        // Задаем лимит ожидания ровно 5 секунд
        struct timeval timeout;
        timeout.tv_sec = 5;
        timeout.tv_usec = 0;

        // Системный вызов select ожидает события ввода с таймаутом
        int ret = select(STDIN_FILENO + 1, &readfds, NULL, NULL, &timeout);

        if (ret == -1) {
            perror("select failed");
            break;
        } else if (ret == 0) {
            // ТАЙМАУТ: 5 секунд прошло, пользователь ничего не ввел
            print_entire_file(fd);
            break;
        }

        // Пользователь успел что-то ввести! Считываем число
        int line_num;
        if (scanf("%d", &line_num) != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }

        if (line_num == 0) {
            printf("Exiting...\n");
            break;
        }

        if (line_num < 1 || line_num > num_lines) {
            fprintf(stderr, "Error: Line number must be between 1 and %d\n", num_lines);
            continue;
        }

        // Выводим запрошенную строку
        LineInfo info = table[line_num - 1];
        if (lseek(fd, info.offset, SEEK_SET) == (off_t)-1) {
            perror("lseek failed");
            continue;
        }

        char *buf = malloc(info.length + 1);
        if (!buf) {
            perror("malloc failed");
            continue;
        }

        ssize_t bytes_read = read(fd, buf, info.length);
        if (bytes_read != info.length) {
            perror("read failed");
            free(buf);
            continue;
        }

        buf[bytes_read] = '\0';
        printf("%s", buf);
        if (bytes_read > 0 && buf[bytes_read - 1] != '\n') {
            printf("\n");
        }

        free(buf);
    }

    free(table);
    close(fd);

    return 0;
}
