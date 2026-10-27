#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {
    // 1. Устанавливаем часовой пояс Калифорнии
    setenv("TZ", "America/Los_Angeles", 1);
    
    // 2. Инициализируем библиотеку времени с новым поясом
    tzset();

    // 3. Получаем текущее время в секундах (UTC)
    time_t now;
    time(&now);

    // 4. Переводим секунды в структуру локального времени Калифорнии
    struct tm *sp = localtime(&now);

    // 5. Выводим дату и время:
    // tm_mon + 1 (месяцы считаются от 0 до 11)
    // tm_year + 1900 (годы считаются от 1900)
    // tzname[sp->tm_isdst] (выбирает PST или PDT в зависимости от летнего времени)
    printf("%02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1,
           sp->tm_mday,
           sp->tm_year + 1900,
           sp->tm_hour,
           sp->tm_min,
           tzname[sp->tm_isdst]);

    return 0;
}
