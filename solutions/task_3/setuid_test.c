/* Задание 3. Установка идентификатора пользователя для доступа к файлу. */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define FILENAME "data.txt"

static void check(void)
{
    FILE *fp;

    printf("real uid = %d, effective uid = %d\n", (int)getuid(), (int)geteuid());

    fp = fopen(FILENAME, "r");
    if (fp == NULL) {
        perror(FILENAME);
    } else {
        printf("%s opened\n", FILENAME);
        fclose(fp);
    }
}

int main(void)
{
    check();

    if (setuid(getuid()) == -1) {
        perror("setuid");
        exit(EXIT_FAILURE);
    }

    check();
    return 0;
}
