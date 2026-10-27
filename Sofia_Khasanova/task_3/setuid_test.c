#include <stdio.h>        // printf, perror
#include <stdlib.h>       // exit, EXIT_FAILURE
#include <unistd.h>       // getuid(), geteuid(), setuid()
#include <sys/types.h>    // типы данных для UID

void r_e_id()
{
    printf("Real UID: %d, Effective UID: %d\n", getuid(), geteuid());
}


int main()
{
    FILE *text, *text_1;

    r_e_id();

    text = fopen("data.txt", "r");

    if (text == NULL)
    {
        perror("Ошибка чтения файла");
        return 1;
    }

    if (setuid(getuid()) == -1)
    {
        perror("setuid is wrong");
        fclose(text);
        return 1;
    }
    
    r_e_id();

    text_1 = fopen("data.txt", "r");

    if (text_1 == NULL)
    {
        perror("Ошибка чтения файла");
        fclose(text);
        return 1;
    }

    fclose(text);
    fclose(text_1);

    return 0;

}
