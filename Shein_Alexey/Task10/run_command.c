#define _XOPEN_SOURCE 600

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    if (argc < 2) {
        fprintf(stderr, "Usage: %s command [arguments...]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Starting command: %s\n", argv[1]);

    if (fflush(stdout) == EOF) {
        perror("fflush");
        return EXIT_FAILURE;
    }

    pid = fork();

    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        int error;

        execvp(argv[1], &argv[1]);

        /* Reached only if execvp() failed. */
        error = errno;
        perror("execvp");
        _exit(error == ENOENT ? 127 : 126);
    }

    while (waitpid(pid, &status, 0) == -1) {
        if (errno == EINTR) {
            continue;
        }

        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        int code = WEXITSTATUS(status);

        printf("[PARENT] Command exited with code: %d\n", code);
        return code;
    }

    if (WIFSIGNALED(status)) {
        printf("[PARENT] Command terminated by signal: %d\n",
               WTERMSIG(status));
        return EXIT_FAILURE;
    }

    fprintf(stderr, "[PARENT] Unexpected child status\n");
    return EXIT_FAILURE;
}
