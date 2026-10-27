#include <stdio.h>
#include <time.h>
#include <stdlib.h>

int main(void) {
    //1. Устанавливаем часовой пояс Калифорнии
    setenv("TZ", "America/Los_Angeles", 1);
    tzset();

    //2. Получаем текущее время
    time_t now;
    time(&now);

    //3. Конвертируем в локальное (с учётом TZ)
    struct tm *sp = localtime(&now);

    //4. Печатаем
    printf("%02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1,      // месяц: 0-11 -> 1-12
           sp->tm_mday,         // день месяца
           sp->tm_year + 1900,  // год: от 1900 
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst]); // PST или PDT

    return 0;
}z