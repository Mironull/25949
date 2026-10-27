#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_ids(void)
{
    printf("Real UID: %ld\n", (long)getuid());
    printf("Effective UID: %ld\n", geteuid());
}

void test_file(void)
{
    FILE *file;
    file = fopen("data.txt", "r");

    if (file == NULL)
    {
        perror("fopen");
        return;
    }

    printf("File opened succeddfully\n");
    fclose(file);
}

int main(void)
{
    printf("Before setuid():\n");
    print_ids();

    printf("Trying to open data.txt:\n");
    test_file();

    printf("\nCalling setuid(getuid())...\n");

    if (setuid(getuid()) == -1)
    {
        perror("setuid");
        return EXIT_FAILURE;
    }

    printf("\nAfter setuid():\n");
    print_ids();

    printf("nAfter setuid():\n");
    print_ids();

    printf("Trying to open data.txt:\n");
    test_file();

    return EXIT_SUCCESS;
}