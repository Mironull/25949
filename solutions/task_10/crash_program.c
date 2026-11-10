/* Test program for task 10: crashes with a segmentation fault. */

#include <stdio.h>

int main(void)
{
    volatile int *p = NULL;

    printf("crash_program: writing through a NULL pointer\n");
    fflush(stdout);
    *p = 42;
    return 0;
}
