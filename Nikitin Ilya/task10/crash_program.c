#include <stdio.h>

int main(void)
{
    int *ptr = NULL;

    printf("Crash program started\n");

    *ptr = 42;

    return 0;
}