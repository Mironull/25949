#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

void print_uids(const char *msg) {
    printf("[%s] Real UID: %ld, Effective UID: %ld\n",
           msg, (long)getuid(), (long)geteuid());
}

void try_open(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (f == NULL) {
        perror("  Open failed");
    } else {
        printf("  SUCCESS: File '%s' opened successfully!\n", filename);
        fclose(f);
    }
}

int main(void) {
    const char *filename = "data.txt";

    printf("=== STAGE 1: Before dropping privileges ===\n");
    print_uids("Start");
    try_open(filename);

    printf("\n=== Dropping privileges: setuid(getuid()) ===\n");
    if (setuid(getuid()) == -1) {
        perror("setuid error");
    }

    printf("\n=== STAGE 2: After dropping privileges ===\n");
    print_uids("After setuid");
    try_open(filename);

    return 0;
}
