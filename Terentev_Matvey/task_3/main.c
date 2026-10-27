#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#define FILENAME "protected_file"

static void print_ids(const char *label)
{
    printf("--- %s ---\n", label);
    printf("Real UID      = %d\n", (int)getuid());
    printf("Effective UID = %d\n", (int)geteuid());
    printf("Real GID      = %d\n", (int)getgid());
    printf("Effective GID = %d\n", (int)getegid());
}

static void try_open(void)
{
    FILE *f = fopen(FILENAME, "r");
    if (f == NULL) {
        perror("fopen");
        return;
    }
    printf("fopen(\"%s\") succeeded\n", FILENAME);
    if (fclose(f) != 0) {
        perror("fclose");
    }
}

int main(void)
{
    print_ids("Before setuid");
    try_open();

    if (setuid(getuid()) == -1) {
        perror("setuid");
        return 1;
    }

    print_ids("After setuid");
    try_open();

    return 0;
}
