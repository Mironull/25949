#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>

typedef struct {
    off_t offset;
    int length;
} LineInfo;

// Глобальные указатели для обработчика сигнала
static char *g_map = NULL;
static size_t g_size = 0;

// Обработчик сигнала таймаута SIGALRM
void alarm_handler(int sig) {
    (void)sig;

    const char msg[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);

    // Выводим всю отображенную память напрямую за один системный вызов write!
    if (g_map != NULL && g_size > 0) {
        write(STDOUT_FILENO, g_map, g_size);
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    // 1. Открываем файл
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open failed");
        return 1;
    }

    // 2. Узнаем точный размер файла через fstat
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat failed");
        close(fd);
        return 1;
    }

    // Проверка на пустой файл (Сценарий 3 из методички)
    if (st.st_size == 0) {
        printf("File is empty.\n");
        close(fd);
        return 0;
    }

    g_size = (size_t)st.st_size;

    // 3. Отображаем файл в память процесса через mmap
    g_map = (char *)mmap(NULL, g_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        return 1;
    }

    // После успешного mmap дескриптор файла можно сразу закрыть!
    close(fd);

    // 4. Построение таблицы строк (простой быстрый цикл по массиву g_map)
    int capacity = 16;
    int num_lines = 0;
    LineInfo *table = malloc(capacity * sizeof(LineInfo));
    if (!table) {
        perror("malloc failed");
        munmap(g_map, g_size);
        return 1;
    }

    off_t line_start = 0;
    int line_len = 0;

    for (size_t i = 0; i < g_size; i++) {
        line_len++;

        if (g_map[i] == '\n') {
            if (num_lines >= capacity) {
                capacity *= 2;
                LineInfo *new_table = realloc(table, capacity * sizeof(LineInfo));
                if (!new_table) {
                    perror("realloc failed");
                    free(table);
                    munmap(g_map, g_size);
                    return 1;
                }
                table = new_table;
            }

            table[num_lines].offset = line_start;
            table[num_lines].length = line_len;
            num_lines++;

            line_start = (off_t)(i + 1);
            line_len = 0;
        }
    }

    // Обработка последней строки, если нет завершающего '\n'
    if (line_len > 0) {
        if (num_lines >= capacity) {
            capacity += 1;
            table = realloc(table, capacity * sizeof(LineInfo));
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = line_len;
        num_lines++;
    }

    // 5. Отладочный вывод таблицы
    printf("--- Debug: Line Table ---\n");
    for (int i = 0; i < num_lines; i++) {
        printf("Line %d: Offset = %ld, Length = %d\n", 
               i + 1, (long)table[i].offset, table[i].length);
    }
    printf("-------------------------\n");

    // 6. Регистрируем будильник SIGALRM
    signal(SIGALRM, alarm_handler);

    // 7. Интерактивный цикл
    while (1) {
        printf("Enter line number (0 to quit, 5 sec timeout): ");
        fflush(stdout);

        alarm(5);

        int line_num;
        int ret = scanf("%d", &line_num);

        alarm(0);

        if (ret != 1) {
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

        // 8. Вывод строки через fwrite прямо из отображенной памяти без буферов и read!
        LineInfo info = table[line_num - 1];
        fwrite(g_map + info.offset, 1, (size_t)info.length, stdout);

        if (info.length > 0 && g_map[info.offset + info.length - 1] != '\n') {
            printf("\n");
        }
    }

    // 9. Очистка ресурсов
    free(table);
    munmap(g_map, g_size); // Освобождаем отображение файла

    return 0;
}
