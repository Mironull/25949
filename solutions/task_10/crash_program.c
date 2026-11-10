/* Тестовая программа для задания 10: падает с Segmentation fault. */

#include <stdio.h>

int main(void)
{
    volatile int *p = NULL;

    printf("crash_program: сейчас запишу по нулевому указателю\n");
    fflush(stdout);
    *p = 42;
    return 0;
}
