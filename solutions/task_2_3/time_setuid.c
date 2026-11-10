/*
 * Задания 2 и 3, объединённые в одну программу.
 *
 * Программа выводит текущее время в Калифорнии (задание 2), а затем печатает
 * реальный и эффективный UID процесса до и после сброса привилегий вызовом
 * setuid(getuid()) (задание 3).
 *
 * Часовой пояс задаётся через TZ=America/Los_Angeles, поэтому переход между
 * PST и PDT учитывается автоматически.
 *
 * Если на исполняемом файле стоит бит setuid, а запускает его другой
 * пользователь, то до сброса эффективный UID равен UID владельца файла,
 * а после сброса становится равным реальному UID запустившего.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

static void print_california_time(void)
{
    time_t now;
    struct tm *sp;

    if (setenv("TZ", "America/Los_Angeles", 1) == -1) {
        perror("setenv");
        exit(EXIT_FAILURE);
    }
    tzset();

    time(&now);
    sp = localtime(&now);
    if (sp == NULL) {
        perror("localtime");
        exit(EXIT_FAILURE);
    }

    printf("Время в Калифорнии: %02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday, sp->tm_year + 1900,
           sp->tm_hour, sp->tm_min, tzname[sp->tm_isdst > 0 ? 1 : 0]);
}

static void print_uids(const char *stage)
{
    printf("%s real uid = %ld, effective uid = %ld\n",
           stage, (long)getuid(), (long)geteuid());
}

int main(void)
{
    print_california_time();

    print_uids("До setuid(getuid()):   ");

    if (setuid(getuid()) == -1) {
        perror("setuid");
        exit(EXIT_FAILURE);
    }

    print_uids("После setuid(getuid()):");
    return 0;
}
