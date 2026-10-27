#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_LINES 1000

typedef struct {
    off_t offset;  // смещение начала строки от начала файла
    size_t length; // длина строки в байтах
} LineInfo;

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    // открытие файла только для чтения через системный вызов open(2)
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("ошибка при открытии файла");
        return 1;
    }

    LineInfo lines[MAX_LINES];
    int line_count = 0;

    char ch;
    off_t line_start = 0; // первая строка начинается с нулевого отступа

    // построение таблицы отступов в файле и длин строк
    while (read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            // lseek(fd, 0L, 1) возвращает текущую позицию указателя в файле
            off_t current_pos = lseek(fd, 0L, 1);
            if (current_pos == -1) {
                perror("ошибка lseek при построении таблицы");
                close(fd);
                return 1;
            }

            lines[line_count].offset = line_start;
            lines[line_count].length = (size_t)(current_pos - line_start);
            line_count++;

            if (line_count >= MAX_LINES) {
                fprintf(stderr, "достигнут максимальный лимит строк\n");
                break;
            }

            line_start = current_pos;
        }
    }

    // обработка последней строки, если файл не заканчивается переводом строки
    off_t file_end = lseek(fd, 0L, 1);
    if (file_end > line_start && line_count < MAX_LINES) {
        lines[line_count].offset = line_start;
        lines[line_count].length = (size_t)(file_end - line_start);
        line_count++;
    }

    // распечатка отладочной таблицы согласно подсказке в задании
    printf("таблица строк файла (всего %d):\n", line_count);
    printf("№\tотступ (offset)\tдлина (length)\n");
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%ld\t\t%lu\n", i + 1, (long)lines[i].offset, (unsigned long)lines[i].length);
    }
    printf("----------------------------------------\n");

    int requested_line;
    char buffer[1024];

    // запрос номера строки и печать через printf(3)
    while (1) {
        printf("введите номер строки (1..%d, 0 для выхода): ", line_count);
        if (scanf("%d", &requested_line) != 1) {
            fprintf(stderr, "некорректный ввод\n");
            break;
        }

        // ввод нулевого номера завершает работу программы
        if (requested_line == 0) {
            break;
        }

        if (requested_line < 1 || requested_line > line_count) {
            printf("ошибка: строки с номером %d не существует\n", requested_line);
            continue;
        }

        int idx = requested_line - 1;

        // позиционирование на начало заданной строки через lseek(2)
        if (lseek(fd, lines[idx].offset, SEEK_SET) == -1) {
            perror("ошибка lseek");
            continue;
        }

        // прочтение точной длины строки через read(2)
        size_t to_read = lines[idx].length;
        if (to_read >= sizeof(buffer)) {
            to_read = sizeof(buffer) - 1;
        }

        ssize_t bytes_read = read(fd, buffer, to_read);
        if (bytes_read == -1) {
            perror("ошибка read");
            continue;
        }

        buffer[bytes_read] = '\0';
        printf("%s", buffer);
        if (buffer[bytes_read - 1] != '\n') {
            printf("\n");
        }
    }

    // закрытие файла через close(2)
    close(fd);
    return 0;
}

/*
printf "one\ntwo three\n4\n" > test5.txt
gcc -Wall -Wextra task5.c -o task5
./task5 test5.txt
*/