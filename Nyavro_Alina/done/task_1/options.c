/*
  options.c - вывод атрибутов процесса в соответствии с опциями.
  Опции обрабатываются в порядке появления справа налево.
  Одну опцию можно указывать несколько раз.
 */

#include <stdlib.h>       // realloc, free, exit, strtol
#include <stdio.h>        // printf, fprintf
#include <unistd.h>       // getopt, getpid, getuid, getcwd
#include <sys/resource.h> // struct rlimit, getrlimit, setrlimit
#include <limits.h>       // PATH_MAX
#include <string.h>       // strdup, strchr
#include <errno.h>        // errno
#include <sys/types.h>    // uid_t, pid_t 

//Список запомненных опций
typedef struct {
    char opt;      //буква опции: i, s, p, u, U, c, C, d, v, V 
    char *arg;     //аргумент для U, C, V; иначе NULL 
} action_t;

action_t actions[256];
int n_actions = 0;

static void add_action(char opt, char *arg) {
    if (n_actions >= 256) {
        fprintf(stderr, "Too many options\n");
        exit(1);
    }
    actions[n_actions].opt = opt;
    actions[n_actions].arg = arg ? strdup(arg) : NULL; 
    //strdup выделяет память и копирует туда содержимое. Возвращает указатель на копию.
    n_actions++;
}

//Действия

// -i: реальные и эффективные UID/GID 
static void do_i(void) {
    printf("Real UID = %d, Effective UID = %d\n", getuid(), geteuid());
    printf("Real GID = %d, Effective GID = %d\n", getgid(), getegid());
}

// -s: стать лидером группы
static void do_s(void) {
    /* setpgid(pid, pgid) 
        pid - для какого процесса менять группу. 0 означает для текущего процесса.
        pgid - ID новой группы. 0 означает создать новую группу, лидером которой будет текущий процесс.
    */
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
    } 
    else {
        printf("Process became group leader, PGID = %d\n", getpgrp());
    }
}

// -p: PID, PPID, PGID 
/*  PID - идентификатор процесса.
    PPID - идентификатор родителя.
    PGID - идентификатор группы процессов.
*/
static void do_p(void) {
    printf("PID  = %d\n", getpid());
    printf("PPID = %d\n", getppid());
    printf("PGID = %d\n", getpgrp());
}

// -u: значение ulimit (RLIMIT_FSIZE - лимит размера файла)
static void do_u(void) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_FSIZE, &rl) == 0) {
        printf("ulimit (RLIMIT_FSIZE): soft = %ld, hard = %ld\n",
               (long)rl.rlim_cur, (long)rl.rlim_max);
    } 
    else {
        perror("getrlimit");
    }
}

// -U<число>: изменить ulimit
static void do_U(const char *arg) {
    if (!arg) {
        fprintf(stderr, "-U requires an argument\n");
        return;
    }
    long val = atol(arg);
    if (val < 0) {
        fprintf(stderr, "Invalid ulimit value: %s\n", arg);
        return;
    }
    struct rlimit rl;
    if (getrlimit(RLIMIT_FSIZE, &rl) == -1) {
        perror("getrlimit");
        return;
    }
    rl.rlim_cur = (rlim_t)val;
    if (setrlimit(RLIMIT_FSIZE, &rl) == -1) {
        perror("setrlimit");
    } 
    else {
        printf("New ulimit = %ld\n", val);
    }
}

// -c: размер core-файла 
static void do_c(void) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == 0) {
        printf("Core size: soft = %ld, hard = %ld\n",
               (long)rl.rlim_cur, (long)rl.rlim_max);
    } 
    else {
        perror("getrlimit");
    }
}

// -C<число>: изменить размер core-файла 
static void do_C(const char *arg) {
    if (!arg) {
        fprintf(stderr, "-C requires an argument\n");
        return;
    }
    long val = atol(arg);
    if (val < 0) {
        fprintf(stderr, "Invalid core size: %s\n", arg);
        return;
    }
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit");
        return;
    }
    rl.rlim_cur = (rlim_t)val;
    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("setrlimit");
    } 
    else {
        printf("New core size = %ld\n", val);
    }
}

// -d: текущая рабочая директория
static void do_d(void) {
    char buf[PATH_MAX]; //массив символов пути
    if (getcwd(buf, sizeof(buf)) == NULL) {
        perror("getcwd");
    } 
    else {
        printf("Current directory: %s\n", buf);
    }
}

extern char **environ;    // Глобальный указатель на массив переменных среды

// -v: переменные среды
static void do_v(void) {
    for (char **e = environ; *e; e++) {
        printf("%s\n", *e);
    }
}

//-Vname=value: установить переменную среды
static void do_V(const char *arg) {
    if (!arg || !strchr(arg, '=')) {
        fprintf(stderr, "-V requires name=value\n");
        return;
    }
    if (putenv((char *)arg) != 0) { //putenv добавляет или меняет переменную среды
        perror("putenv");
    } 
    else {
        printf("Set env: %s\n", arg);
    }
}

/* ---------- main ---------- */

int main(int argc, char *argv[]) {
    int opt;
    opterr = 0;  // отключаем автосообщения getopt 

    /* ':' в начале - сами обрабатываем отсутствие аргумента.
       ':' после U, C, V - опция требует аргумент. */
    while ((opt = getopt(argc, argv, ":ispuU:cC:dvV:")) != -1) {
        switch (opt) {
            case 'i': add_action('i', NULL); break;
            case 's': add_action('s', NULL); break;
            case 'p': add_action('p', NULL); break;
            case 'u': add_action('u', NULL); break;
            case 'U': add_action('U', optarg); break;
            case 'c': add_action('c', NULL); break;
            case 'C': add_action('C', optarg); break;
            case 'd': add_action('d', NULL); break;
            case 'v': add_action('v', NULL); break;
            case 'V': add_action('V', optarg); break;
            case ':':
                fprintf(stderr, "Option -%c requires an argument\n", optopt);
                return 1;
            case '?':
            default:
                fprintf(stderr, "Unknown option: -%c\n", optopt);
                return 1;
        }
    }

    if (n_actions == 0) {
        printf("No options provided.\n");
        return 0;
    }

    // Выполняем в порядке, обратном порядку появления (справа налево) 
    for (int i = n_actions - 1; i >= 0; i--) {
        switch (actions[i].opt) {
            case 'i': do_i(); break;
            case 's': do_s(); break;
            case 'p': do_p(); break;
            case 'u': do_u(); break;
            case 'U': do_U(actions[i].arg); break;
            case 'c': do_c(); break;
            case 'C': do_C(actions[i].arg); break;
            case 'd': do_d(); break;
            case 'v': do_v(); break;
            case 'V': do_V(actions[i].arg); break;
        }
    }

    // Освобождаем память 
    for (int i = 0; i < n_actions; i++) {
        free(actions[i].arg);
    }
    return 0;
}