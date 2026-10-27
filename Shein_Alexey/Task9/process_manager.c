#define _XOPEN_SOURCE 600

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;
    int no_wait = 0;
    int result = EXIT_SUCCESS;

    if (argc != 2 &&
        !(argc == 3 && strcmp(argv[2], "--no-wait") == 0)) {
        fprintf(stderr, "Usage: %s filename [--no-wait]\n",
                argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 3) {
        no_wait = 1;
    }

    printf("[PARENT] Starting. PID: %ld\n", (long)getpid());

    /* Do not duplicate buffered output across fork(). */
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
        printf("[CHILD] PID: %ld. Running cat.\n",
               (long)getpid());

        if (fflush(stdout) == EOF) {
            perror("fflush");
            _exit(EXIT_FAILURE);
        }

        execlp("cat", "cat", argv[1], (char *)NULL);

        /* Executed only if execlp() failed. */
        perror("execlp cat");
        _exit(127);
    }

    printf("[PARENT] Child PID: %ld\n", (long)pid);
    printf("[PARENT] Printing text while child can run.\n");

    if (fflush(stdout) == EOF) {
        perror("fflush");
        result = EXIT_FAILURE;
    }

    if (no_wait) {
        printf("[PARENT] Last parent line WITHOUT waiting.\n");
        return result;
    }

    while (waitpid(pid, &status, 0) == -1) {
        if (errno == EINTR) {
            continue;
        }

        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status)) {
        printf("[PARENT] Child exit code: %d\n",
               WEXITSTATUS(status));

        if (WEXITSTATUS(status) != 0) {
            result = EXIT_FAILURE;
        }
    } else if (WIFSIGNALED(status)) {
        printf("[PARENT] Child terminated by signal: %d\n",
               WTERMSIG(status));
        result = EXIT_FAILURE;
    } else {
        fprintf(stderr, "[PARENT] Unexpected child status\n");
        result = EXIT_FAILURE;
    }

    printf("[PARENT] Child has finished. "
           "This is the last parent line.\n");

    return result;
}
