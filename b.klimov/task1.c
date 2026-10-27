#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <errno.h>

extern char **environ;

int main(int argc, char *argv[]) {
    int opt;
    char cwd[1024];
    struct rlimit rl;
    long new_ulimit;
    long new_csize;

    while ((opt = getopt(argc, argv, "ispuU:c:C:dvV:")) != -1) {
        switch (opt) {
            case 'i':
                printf("UID: real=%ld, effective=%ld | GID: real=%ld, effective=%ld\n",
                       (long)getuid(), (long)geteuid(), (long)getgid(), (long)getegid());
                break;

            case 's':
                if (setpgid(0, 0) == -1) {
                    perror("setpgid failed");
                } else {
                    printf("Process became group leader. PGID=%ld\n", (long)getpgrp());
                }
                break;

            case 'p':
                printf("PID=%ld, PPID=%ld, PGID=%ld\n",
                       (long)getpid(), (long)getppid(), (long)getpgrp());
                break;

            case 'u': {
                long lim = ulimit(UL_GETFSIZE, 0);
                if (lim == -1) {
                    perror("ulimit GET failed");
                } else if (lim >= 0x7FFFFFFFFFFFFFFF / 512L) {
                    printf("ulimit: unlimited\n");
                } else {
                    printf("ulimit: %ld blocks (%ld bytes)\n", lim, lim * 512L);
                }
                break;
            }

            case 'U':
                new_ulimit = atol(optarg);
                if (new_ulimit <= 0) {
                    fprintf(stderr, "Invalid ulimit value: %s\n", optarg);
                } else if (ulimit(UL_SETFSIZE, new_ulimit) == -1) {
                    perror("ulimit SET failed");
                } else {
                    printf("ulimit successfully set to %ld blocks\n", new_ulimit);
                }
                break;

            case 'c':
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("getrlimit failed");
                } else {
                    printf("Core file size limit: soft=%lu bytes, hard=%lu bytes\n",
                           (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
                }
                break;

            case 'C':
                new_csize = atol(optarg);
                if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("getrlimit failed");
                    break;
                }
                rl.rlim_cur = (rlim_t)new_csize;
                if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                    perror("setrlimit failed");
                } else {
                    printf("Core file soft limit set to %lu bytes\n", (unsigned long)rl.rlim_cur);
                }
                break;

            case 'd':
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Current working directory: %s\n", cwd);
                } else {
                    perror("getcwd failed");
                }
                break;

            case 'v':
                printf("Environment variables:\n");
                for (char **env = environ; *env != NULL; ++env) {
                    printf("  %s\n", *env);
                }
                break;

            case 'V':
                if (putenv(optarg) != 0) {
                    perror("putenv failed");
                } else {
                    printf("Environment updated: %s\n", optarg);
                }
                break;

            case '?':
                break;

            default:
                break;
        }
    }

    return 0;
}


//
//gcc -Wall -Wextra task1.c -o task1
//
//

//./task1
//./task1 -z
//./task1 -i -p -d
//./task1 -ipd
//./task1 -p -s -p
//./task1 -U -1
//./task1 -U 999999999999
//./task1 -u -U 2048 -u
//./task1 -c -C 0 -c
//./task1 -VOS_TEST_VARIABLE=12345 -v | grep OS_TEST_VARIABLE