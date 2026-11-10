/*
 * Задание 10. Код завершения команды.
 *
 * Программа запускает команду из первого аргумента в виде порождённого
 * процесса и передаёт ей все остальные аргументы. Затем она ждёт завершения
 * процесса и печатает его код завершения, а если процесс убит сигналом,
 * то номер и описание сигнала.
 *
 * Запуск: ./run_command команда [аргументы...]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    pid_t pid;
    int status;

    if (argc < 2) {
        fprintf(stderr, "Использование: %s команда [аргументы...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Запускаю команду: %s\n", argv[1]);
    fflush(stdout);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* argv[1] — имя команды, &argv[1] — её argv, который кончается NULL. */
        execvp(argv[1], &argv[1]);
        perror("execvp");
        _exit(127);   /* 127 в Unix означает «команда не найдена». */
    }

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf("[PARENT] Команда завершилась нормально с кодом %d\n",
               WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("[PARENT] Команда убита сигналом %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));
    }
    return 0;
}
