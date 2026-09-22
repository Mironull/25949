/*
 * Задание 1. Вывод атрибутов процесса в соответствии с указанными опциями.
 *
 * Ключевое требование: опции обрабатываются в порядке своего появления
 * СПРАВА НАЛЕВО. getopt(3C) разбирает командную строку слева направо, поэтому
 * разбор и исполнение разнесены: сначала все опции складываются в массив,
 * затем массив обходится с конца.
 *
 * Сборка: make
 */

#include <stdio.h>          /* printf, fprintf, perror                     */
#include <stdlib.h>         /* malloc, realloc, free, strtol, exit         */
#include <string.h>         /* strchr                                      */
#include <errno.h>          /* errno, ERANGE                               */
#include <unistd.h>         /* getopt, optarg, optopt, getpid, getuid, ... */
#include <ulimit.h>         /* ulimit(2), UL_GETFSIZE, UL_SETFSIZE         */
#include <sys/types.h>      /* uid_t, gid_t, pid_t                         */
#include <sys/resource.h>   /* struct rlimit, getrlimit, setrlimit          */

extern char **environ;      /* массив строк вида "ИМЯ=значение"            */

/*
 * Ведущее двоеточие в строке опций переводит getopt в "тихий" режим и
 * заставляет его возвращать ':' при отсутствии обязательного аргумента
 * (иначе пропущенный аргумент неотличим от неизвестной опции).
 */
#define OPTSTRING ":ispuU:cC:dvV:"

#define BLOCK_SIZE 512      /* ulimit(2) меряет размер файла в блоках по 512 байт */

/* Одна распознанная опция: сам символ и её аргумент (NULL, если аргумента нет). */
typedef struct {
    int   opt;
    char *arg;              /* указывает внутрь argv[], копировать не нужно */
} opt_entry;

/* ------------------------------------------------------------------ */
/* Разбор числового аргумента                                          */
/* ------------------------------------------------------------------ */

/*
 * Переводит строку в неотрицательное long.
 * Возвращает 0 при успехе, -1 при любой ошибке (не число, мусор в хвосте,
 * переполнение, отрицательное значение).
 */
static int parse_limit(const char *s, long *out)
{
    char *end;
    long  value;

    errno = 0;
    value = strtol(s, &end, 10);

    if (end == s) {                         /* вообще не число: "abc"      */
        fprintf(stderr, "Ошибка: '%s' не является числом\n", s);
        return -1;
    }
    if (*end != '\0') {                     /* мусор в хвосте: "123abc"    */
        fprintf(stderr, "Ошибка: лишние символы в '%s'\n", s);
        return -1;
    }
    if (errno == ERANGE) {                  /* не влезло в long            */
        fprintf(stderr, "Ошибка: значение '%s' вне диапазона\n", s);
        return -1;
    }
    if (value < 0) {                        /* краевой случай из задания: -U-5 */
        fprintf(stderr, "Ошибка: значение '%s' должно быть неотрицательным\n", s);
        return -1;
    }

    *out = value;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Обработчики опций                                                   */
/* ------------------------------------------------------------------ */

/* -i : реальные и эффективные идентификаторы пользователя и группы. */
static void print_ids(void)
{
    printf("-i: real UID = %ld, effective UID = %ld\n",
           (long)getuid(), (long)geteuid());
    printf("    real GID = %ld, effective GID = %ld\n",
           (long)getgid(), (long)getegid());
}

/*
 * -s : процесс становится лидером группы.
 * setpgid(0, 0) — "для текущего процесса сделать PGID равным его PID".
 * Ошибка EPERM здесь нормальна: процесс уже лидер сессии либо является
 * потомком, уже выполнившим exec.
 */
static void become_group_leader(void)
{
    if (setpgid(0, 0) == -1) {
        perror("-s: setpgid");
        return;
    }
    printf("-s: процесс стал лидером группы, PGID = %ld\n", (long)getpgrp());
}

/* -p : идентификаторы процесса, родителя и группы процессов. */
static void print_pids(void)
{
    printf("-p: PID = %ld, PPID = %ld, PGID = %ld\n",
           (long)getpid(), (long)getppid(), (long)getpgrp());
}

/*
 * -u : печать ulimit.
 * ulimit(2) в SVR4 — это предел на размер файла, который процесс может
 * создать, выраженный в блоках по 512 байт.
 */
static void print_ulimit(void)
{
    long limit;

    errno = 0;
    limit = ulimit(UL_GETFSIZE);
    if (limit == -1 && errno != 0) {
        perror("-u: ulimit(UL_GETFSIZE)");
        return;
    }

    printf("-u: ulimit (file size) = %ld блоков = %ld байт\n",
           limit, limit * BLOCK_SIZE);
}

/* -Unew_ulimit : изменение ulimit. Понизить может любой, повысить — только root. */
static void set_ulimit(const char *arg)
{
    long value;

    if (parse_limit(arg, &value) == -1) {
        fprintf(stderr, "-U: значение не изменено\n");
        return;
    }

    if (ulimit(UL_SETFSIZE, value) == -1) {
        perror("-U: ulimit(UL_SETFSIZE)");
        return;
    }

    printf("-U: ulimit установлен в %ld блоков (%ld байт)\n",
           value, value * BLOCK_SIZE);
}

/* -c : размер core-файла, который может быть создан. */
static void print_core_size(void)
{
    struct rlimit rl;

    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("-c: getrlimit(RLIMIT_CORE)");
        return;
    }

    if (rl.rlim_cur == RLIM_INFINITY)
        printf("-c: core file size = unlimited");
    else
        printf("-c: core file size = %lld байт", (long long)rl.rlim_cur);

    if (rl.rlim_max == RLIM_INFINITY)
        printf(" (жёсткий предел: unlimited)\n");
    else
        printf(" (жёсткий предел: %lld байт)\n", (long long)rl.rlim_max);
}

/*
 * -Csize : изменение размера core-файла.
 * Меняется только мягкий предел (rlim_cur); жёсткий сохраняем, поднять его
 * непривилегированный процесс всё равно не может.
 */
static void set_core_size(const char *arg)
{
    struct rlimit rl;
    long          value;

    if (parse_limit(arg, &value) == -1) {
        fprintf(stderr, "-C: значение не изменено\n");
        return;
    }

    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("-C: getrlimit(RLIMIT_CORE)");
        return;
    }

    rl.rlim_cur = (rlim_t)value;

    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("-C: setrlimit(RLIMIT_CORE)");
        return;
    }

    printf("-C: core file size установлен в %ld байт\n", value);
}

/* -d : текущая рабочая директория. */
static void print_cwd(void)
{
    char *cwd = getcwd(NULL, 0);    /* SVR4/illumos: при buf == NULL память выделяется сам */

    if (cwd == NULL) {
        perror("-d: getcwd");
        return;
    }

    printf("-d: cwd = %s\n", cwd);
    free(cwd);
}

/* -v : переменные среды и их значения. */
static void print_env(void)
{
    char **p;

    printf("-v: переменные среды:\n");
    for (p = environ; *p != NULL; p++)
        printf("    %s\n", *p);
}

/*
 * -Vname=value : внести переменную в среду или изменить существующую.
 *
 * putenv(3C) НЕ копирует строку — она становится частью среды. Поэтому
 * передавать сюда локальный буфер нельзя. Здесь передаётся optarg, который
 * указывает внутрь argv[]: эта память живёт всё время работы процесса.
 */
static void set_env(char *arg)
{
    if (strchr(arg, '=') == NULL) {
        fprintf(stderr, "-V: ожидается формат name=value, получено '%s'\n", arg);
        return;
    }

    if (putenv(arg) != 0) {
        perror("-V: putenv");
        return;
    }

    printf("-V: установлено %s\n", arg);
}

/* ------------------------------------------------------------------ */
/* Диспетчер                                                           */
/* ------------------------------------------------------------------ */

static void execute(const opt_entry *e)
{
    switch (e->opt) {
    case 'i': print_ids();            break;
    case 's': become_group_leader();  break;
    case 'p': print_pids();           break;
    case 'u': print_ulimit();         break;
    case 'U': set_ulimit(e->arg);     break;
    case 'c': print_core_size();      break;
    case 'C': set_core_size(e->arg);  break;
    case 'd': print_cwd();            break;
    case 'v': print_env();            break;
    case 'V': set_env(e->arg);        break;
    default:  /* сюда не попадаем: неизвестные опции отсеяны при разборе */
        break;
    }
}

/* ------------------------------------------------------------------ */
/* main                                                                */
/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    opt_entry *opts     = NULL;     /* накопитель распознанных опций */
    size_t     count    = 0;        /* сколько опций уже накоплено   */
    size_t     capacity = 0;        /* сколько влезает без realloc   */
    int        c;
    int        i;

    while ((c = getopt(argc, argv, OPTSTRING)) != -1) {

        if (c == '?') {                         /* неизвестная опция */
            fprintf(stderr, "Неизвестная опция: -%c\n", optopt);
            continue;                           /* остальные всё равно разбираем */
        }
        if (c == ':') {                         /* опция есть, аргумента нет */
            fprintf(stderr, "Опция -%c требует аргумент\n", optopt);
            continue;
        }

        /* Растим массив вдвое, когда он заполнился. */
        if (count == capacity) {
            size_t     new_cap = (capacity == 0) ? 8 : capacity * 2;
            opt_entry *tmp     = realloc(opts, new_cap * sizeof(*opts));

            if (tmp == NULL) {
                perror("realloc");
                free(opts);
                return EXIT_FAILURE;
            }
            opts     = tmp;
            capacity = new_cap;
        }

        opts[count].opt = c;
        opts[count].arg = optarg;   /* для опций без аргумента getopt даёт NULL */
        count++;
    }

    if (count == 0) {
        printf("Опции не заданы.\n");
        printf("Использование: %s [-ispucdv] [-U блоки] [-C байты] [-V имя=значение]\n",
               argv[0]);
        free(opts);
        return EXIT_SUCCESS;
    }

    /* Главное требование задания: обход массива с конца. */
    printf("Распознано опций: %lu. Порядок исполнения — справа налево.\n\n",
           (unsigned long)count);

    for (i = (int)count - 1; i >= 0; i--)
        execute(&opts[i]);

    free(opts);
    return EXIT_SUCCESS;
}
