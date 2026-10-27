#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LINES 1000

typedef struct {
    off_t offset;  // смещение начала строки от начала файла
    size_t length; // длина строки в байтах
} LineInfo;

int global_fd = -1; // дескриптор файла для доступа из обработчика сигнала

// обработчик сигнала таймера sigalrm при истечении 5 секунд
void on_alarm(int sig) {
    (void)sig;
    char dump_buf[1024];
    ssize_t n;

    printf("\nвремя истекло (5 секунд). вывод всего файла:\n");

    // позиционирование в начало файла
    if (lseek(global_fd, 0L, SEEK_SET) == -1) {
        perror("ошибка lseek в обработчике");
        exit(1);
    }

    // чтение и вывод всего содержимого файла
    while ((n = read(global_fd, dump_buf, sizeof(dump_buf))) > 0) {
        if (write(STDOUT_FILENO, dump_buf, (size_t)n) == -1) {
            perror("ошибка write");
            exit(1);
        }
    }

    close(global_fd);
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    global_fd = open(argv[1], O_RDONLY);
    if (global_fd == -1) {
        perror("ошибка при открытии файла");
        return 1;
    }

    LineInfo lines[MAX_LINES];
    int line_count = 0;

    char ch;
    off_t line_start = 0; // первая строка всегда начинается с 0

    // построение таблицы отступов с использованием lseek(fd, 0L, 1)
    while (read(global_fd, &ch, 1) == 1) {
        if (ch == '\n') {
            // lseek(fd, 0L, 1) возвращает текущую позицию указателя в файле
            off_t current_pos = lseek(global_fd, 0L, 1);
            if (current_pos == -1) {
                perror("ошибка lseek при построении таблицы");
                close(global_fd);
                return 1;
            }

            lines[line_count].offset = line_start;
            lines[line_count].length = (size_t)(current_pos - line_start);
            line_count++;

            if (line_count >= MAX_LINES) {
                fprintf(stderr, "достигнут лимит строк\n");
                break;
            }

            line_start = current_pos;
        }
    }

    // проверка на последнюю строку без перевода строки '\n' в конце
    off_t file_end = lseek(global_fd, 0L, 1);
    if (file_end > line_start && line_count < MAX_LINES) {
        lines[line_count].offset = line_start;
        lines[line_count].length = (size_t)(file_end - line_start);
        line_count++;
    }

    // распечатка отладочной таблицы отступов согласно подсказке в задании
    printf("таблица строк файла (всего %d):\n", line_count);
    printf("№\tотступ (offset)\tдлина (length)\n");
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%ld\t\t%lu\n", i + 1, (long)lines[i].offset, (unsigned long)lines[i].length);
    }
    printf("----------------------------------------\n");

    // регистрация обработчика таймера
    signal(SIGALRM, on_alarm);

    int requested_line;
    char buffer[1024];

    while (1) {
        printf("введите номер строки (1..%d, 0 для выхода, 5 сек): ", line_count);
        fflush(stdout);

        // старт таймера на 5 секунд перед вводом
        alarm(5);

        if (scanf("%d", &requested_line) != 1) {
            break;
        }

        // сброс таймера при успешном вводе
        alarm(0);

        if (requested_line == 0) {
            break;
        }

        if (requested_line < 1 || requested_line > line_count) {
            printf("ошибка: строки с номером %d не существует\n", requested_line);
            continue;
        }

        int idx = requested_line - 1;

        // позиционирование на точное смещение строки
        if (lseek(global_fd, lines[idx].offset, SEEK_SET) == -1) {
            perror("ошибка lseek");
            continue;
        }

        // чтение точной длины строки
        size_t to_read = lines[idx].length;
        if (to_read >= sizeof(buffer)) {
            to_read = sizeof(buffer) - 1;
        }

        ssize_t bytes_read = read(global_fd, buffer, to_read);
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

    alarm(0);
    close(global_fd);
    return 0;
}

/*
printf "first line\nsecond line\nthird line\n" > test6.txt
gcc -Wall -Wextra task6.c -o task6
./task6 test6.txt
*/