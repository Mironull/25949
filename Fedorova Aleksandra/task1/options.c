#include <stdlib.h>       // realloc, free, exit, strtol
#include <stdio.h>        // printf, fprintf
#include <unistd.h>       // getopt, getpid, getuid, getcwd
#include <sys/resource.h> // struct rlimit, getrlimit, setrlimit
#include <limits.h>       // PATH_MAX
#include <string.h>       // strdup, strchr
#include <errno.h>        // errno

extern char **environ;

typedef struct {
    char opt;
    char *arg;
} opt_t;

int main(int argc, char *argv[]) {
    // All options: s, p, u, d don't need arguments
    char *options = "ispuU:cC:dvV:";
    opt_t opts[256];
    int n = 0;
    int c;

    // Parse options
    while ((c = getopt(argc, argv, options)) != -1) {
        if (c == '?') {
            printf("Invalid option: %c\n", optopt);
            return 1;
        }
        opts[n].opt = (char)c;
        opts[n].arg = optarg ? strdup(optarg) : NULL;
        n++;
    }

    for (int i = n - 1; i >= 0; i--) {
        char o = opts[i].opt;
        char *a = opts[i].arg;

        switch (o) {
            case 'i': //Печатает реальные и эффективные идентификаторы пользователя и группы.
                printf("Option -i detected\n");
                printf("uid=%d, euid=%d, gid=%d, egid=%d\n", getuid(), geteuid(), getgid(), getegid());
                break;
            case 's': //Процесс становится лидером группы. Подсказка: смотри setpgid(2).
                printf("Option -s detected\n");
                if (setpgid(0, 0) == -1)
                    perror("setpgid");
                else
                    printf("pgid=%d\n", getpgrp());
                break;
            case 'p': //Печатает идентификаторы процесса, процесса-родителя и группы процессов.
                printf("Option -p detected\n");
                printf("pid=%d, ppid=%d, pgid=%d\n", getpid(), getppid(), getpgrp());
                break;
            case 'u': //Печатает значение ulimit
                printf("Option -u detected\n");
                {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_NOFILE, &rl) == 0)
                        printf("soft=%lu, hard=%lu\n",
                               (unsigned long)rl.rlim_cur, (unsigned long)rl.rlim_max);
                    else
                        perror("getrlimit");
                }
                break;
            case 'U': //Изменяет значение ulimit. Подсказка: смотри atol(3C) на странице руководства strtol(3C)
                printf("Option -U with value: %s\n", optarg);
                {
                    errno = 0;
                    char *end;
                    long val = strtol(a, &end, 10);
                    if (errno != 0 || *end != '\0' || val < 0) {
                        printf("Invalid value for -U: %s\n", a);
                        break;
                    }
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
                        perror("getrlimit");
                        break;
                    }
                    rl.rlim_cur = (rlim_t)val;
                    if (setrlimit(RLIMIT_NOFILE, &rl) == -1)
                        perror("setrlimit");
                    else
                        printf("ulimit set to %ld\n", val);
                }
                break;
            case 'c': //Печатает размер в байтах core-файла, который может быть создан.
                printf("Option -c detected\n");
                {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_CORE, &rl) == 0)
                        printf("core size=%lu\n", (unsigned long)rl.rlim_cur);
                    else
                        perror("getrlimit");
                }
                break;
            case 'C': //Изменяет размер core-файла
                printf("Option -C with value: %s\n", optarg);
                {
                    errno = 0;
                    char *end;
                    long val = strtol(a, &end, 10);
                    if (errno != 0 || *end != '\0' || val < 0) {
                        printf("Invalid value for -C: %s\n", a);
                        break;
                    }
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("getrlimit");
                        break;
                    }
                    rl.rlim_cur = (rlim_t)val;
                    if (setrlimit(RLIMIT_CORE, &rl) == -1)
                        perror("setrlimit");
                    else
                        printf("core size set to %ld\n", val);
                }
                break;
            case 'd': //Печатает текущую рабочую директорию
                printf("Option -d detected\n");
                {
                    char buf[PATH_MAX];
                    if (getcwd(buf, sizeof(buf)) != NULL)
                        printf("cwd=%s\n", buf);
                    else
                        perror("getcwd");
                }
                break;
            case 'v': //Распечатывает переменные среды и их значения
                printf("Option -v detected\n");
                for (char **e = environ; *e; e++)
                    printf("%s\n", *e);
                break;
            case 'V': //Вносит новую переменную в среду или изменяет значение существующей переменной.
                printf("Option -V with value: %s\n", optarg);
                if (putenv(a) == 0)
                    printf("set %s\n", a);
                else
                    perror("putenv");
                break;
        }
    }

    for (int i = 0; i < n; i++)
        if (opts[i].arg && opts[i].opt != 'V')
            free(opts[i].arg);

    return 0;
}
