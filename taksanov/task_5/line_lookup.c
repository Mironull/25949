#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

// Структура для хранения информации о строке
typedef struct {
    off_t offset;  // Смещение от начала файла в байтах
    int length;    // Длина строки в байтах (включая символ '\n')
} LineInfo;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // 1. Открываем файл через системный вызов open
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return 1;
    }

    // 2. Первый проход: построение таблицы строк
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

    // Читаем побайтно для точного подсчета смещений
    while (read(fd, &ch, 1) == 1) {
        current_pos++;
        line_len++;

        if (ch == '\n') {
            // Динамически расширяем таблицу при необходимости
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

    // Если последняя строка не заканчивается на '\n', добавляем ее
    if (line_len > 0) {
        if (num_lines >= capacity) {
            capacity += 1;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = line_len;
        num_lines++;
    }

    // 3. Отладочный вывод таблицы
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", 
               i + 1, (long)table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // 4. Интерактивный запрос номера строки
    int line_num;
    while (1) {
        printf("Enter line number (0 to quit): ");
        if (scanf("%d", &line_num) != 1) {
            // Очищаем некорректный ввод
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

        // 5. Позиционируемся через lseek и читаем точную длину строки
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

        // Обязательно добавляем завершающий ноль для корректного вывода
        buf[bytes_read] = '\0';
        printf("%s", buf);

        // Если в конце строки не было '\n', добавляем его для красоты
        if (bytes_read > 0 && buf[bytes_read - 1] != '\n') {
            printf("\n");
        }

        free(buf);
    }

    // 6. Очистка ресурсов
    free(table);
    close(fd);

    return 0;
}
