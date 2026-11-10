/* Test program for task 10: exits with the code given as an argument. */

#include <stdlib.h>

int main(int argc, char **argv)
{
    return (argc > 1) ? atoi(argv[1]) : 0;
}
