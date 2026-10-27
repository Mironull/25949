#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <signal.h>

#define MAX_LINES 1000

typedef struct {
    off_t offset;  // смещение начала строки от начала файла
    size_t length; // длина строки в байтах
} LineInfo;

// глобальные переменные для доступа из обработчика сигнала таймера
char *file_data = MAP_FAILED;
size_t file_size = 0;

// обработчик таймаута: печать файла напрямую из памяти без вызова write(2)
void on_alarm(int sig) {
    (void)sig;
    printf("\nвремя истекло (5 секунд). вывод всего файла из памяти:\n");

    // вывод всего отображенного региона через библиотечный поток stdout
    if (file_data != MAP_FAILED && file_size > 0) {
        fwrite(file_data, 1, file_size, stdout);
        if (file_data[file_size - 1] != '\n') {
            putchar('\n');
        }
    }

    if (file_data != MAP_FAILED) {
        munmap(file_data, file_size);
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "использование: %s <имя_файла>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("ошибка open");
        return 1;
    }

    // получение размера файла через fstat
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("ошибка fstat");
        close(fd);
        return 1;
    }

    file_size = (size_t)st.st_size;

    // обработка пустого файла
    if (file_size == 0) {
        printf("файл пуст\n");
        close(fd);
        return 0;
    }

    // отображение файла в виртуальную память процесса
    file_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED, fd, 0);
    if (file_data == MAP_FAILED) {
        perror("ошибка mmap");
        close(fd);
        return 1;
    }

    // после успешного mmap дескриптор можно сразу закрыть
    close(fd);

    LineInfo lines[MAX_LINES];
    int line_count = 0;
    off_t line_start = 0;

    // сканирование строк прямо по указателям в памяти без вызовов read/lseek
    for (size_t i = 0; i < file_size; i++) {
        if (file_data[i] == '\n') {
            lines[line_count].offset = line_start;
            lines[line_count].length = (size_t)(i - line_start + 1);
            line_count++;

            if (line_count >= MAX_LINES) {
                fprintf(stderr, "достигнут лимит строк\n");
                break;
            }

            line_start = (off_t)(i + 1);
        }
    }

    // учет последней строки без завершающего '\n'
    if ((size_t)line_start < file_size && line_count < MAX_LINES) {
        lines[line_count].offset = line_start;
        lines[line_count].length = file_size - (size_t)line_start;
        line_count++;
    }

    // вывод построенной таблицы отступов
    printf("таблица строк файла (всего %d):\n", line_count);
    printf("№\tотступ (offset)\tдлина (length)\n");
    for (int i = 0; i < line_count; i++) {
        printf("%d\t%ld\t\t%lu\n", i + 1, (long)lines[i].offset, (unsigned long)lines[i].length);
    }
    printf("----------------------------------------\n");

    signal(SIGALRM, on_alarm);

    int requested_line;

    while (1) {
        printf("введите номер строки (1..%d, 0 для выхода, 5 сек): ", line_count);
        fflush(stdout);

        alarm(5);

        if (scanf("%d", &requested_line) != 1) {
            break;
        }

        alarm(0);

        if (requested_line == 0) {
            break;
        }

        if (requested_line < 1 || requested_line > line_count) {
            printf("ошибка: строки с номером %d не существует\n", requested_line);
            continue;
        }

        int idx = requested_line - 1;

        // вывод запрошенной строки напрямую из памяти через fwrite без read/lseek/write
        fwrite(file_data + lines[idx].offset, 1, lines[idx].length, stdout);

        // если в конце строки файла нет перевода строки, добавляем его для читаемости
        if (file_data[lines[idx].offset + lines[idx].length - 1] != '\n') {
            putchar('\n');
        }
    }

    alarm(0);
    munmap(file_data, file_size);
    return 0;
}
/*
printf "first line\nsecond line\nthird line\n" > test7.txt
gcc -Wall -Wextra task7.c -o task7
./task7 test7.txt
strace -e trace=read,lseek,write,mmap ./task7 test7.txt
*/