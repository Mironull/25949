#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

// Функция из Задачи 2: Время в Калифорнии
void print_california_time(void) {
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();
    time_t now;
    time(&now);
    struct tm *sp = localtime(&now);
    if (sp != NULL) {
        printf(">>> [УСПЕХ] Время в Калифорнии: %02d/%02d/%04d %02d:%02d %s\n",
               sp->tm_mon + 1,
               sp->tm_mday,
               sp->tm_year + 1900,
               sp->tm_hour,
               sp->tm_min,
               tzname[sp->tm_isdst]);
    }
}

void print_uids(const char *stage) {
    printf("[%s] Real UID: %d, Effective UID: %d\n", stage, getuid(), geteuid());
}

// Проверяем доступ: если открылся — печатаем время, если нет — отказ
void check_access_and_run(const char *filename, const char *stage) {
    printf("\nПроверка доступа к '%s' (%s):\n", filename, stage);
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        fprintf(stderr, ">>> [ОТКАЗ] Доступ запрещен: ");
        perror("");
        printf(">>> Время в Калифорнии узнать нельзя: нет прав доступа к файлу!\n");
    } else {
        printf(">>> Доступ получен! Файл успешно прочитан.\n");
        fclose(f);
        // Вызываем время в Калифорнии
        print_california_time();
    }
}

int main(void) {
    const char *filename = "data.txt";

    printf("=== 1. ДО СБРОСА ПРИВИЛЕГИЙ ===\n");
    print_uids("Старт");
    check_access_and_run(filename, "Попытка 1");

    printf("\n--------------------------------------------------\n");
    printf("--> Сбрасываем привилегии: setuid(getuid())...\n");
    if (setuid(getuid()) == -1) {
        perror("setuid");
    }
    printf("--------------------------------------------------\n");

    printf("\n=== 2. ПОСЛЕ СБРОСА ПРИВИЛЕГИЙ ===\n");
    print_uids("Финиш");
    check_access_and_run(filename, "Попытка 2");

    return 0;
}
