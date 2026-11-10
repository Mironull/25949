/*
 * Task 10. Exit status of a command.
 *
 * The program runs the command given as the first argument as a child
 * process and passes all remaining arguments to it. Then it waits for the
 * child and prints its exit status, or the signal number and description
 * if the child was killed by a signal.
 *
 * Usage: ./run_command command [args...]
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
        fprintf(stderr, "Usage: %s command [args...]\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Starting command: %s\n", argv[1]);
    fflush(stdout);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        /* argv[1] is the command name, &argv[1] is its NULL-terminated argv. */
        execvp(argv[1], &argv[1]);
        perror("execvp");
        _exit(127);   /* 127 traditionally means "command not found". */
    }

    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf("[PARENT] Command exited normally with code: %d\n",
               WEXITSTATUS(status));
    } else if (WIFSIGNALED(status)) {
        printf("[PARENT] Command terminated by signal: %d (%s)\n",
               WTERMSIG(status), strsignal(WTERMSIG(status)));
    }
    return 0;
}
