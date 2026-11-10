/* Тестовая программа для задания 10: завершается с кодом из аргумента. */

#include <stdlib.h>

int main(int argc, char **argv)
{
    return (argc > 1) ? atoi(argv[1]) : 0;
}
