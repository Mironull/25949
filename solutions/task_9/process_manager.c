/*
 * Task 9. Creating two processes.
 *
 * The program creates a child process that runs cat(1) on a long file.
 * Meanwhile the parent prints its own text with printf(3).
 *
 * By default the parent waits for the child with waitpid(2), so its last
 * line is always printed after the whole output of cat (second part of the
 * task). With the -n option the parent does not wait, and its last line may
 * appear anywhere in the middle of the cat output (first part of the task).
 *
 * Usage: ./process_manager [-n] file
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static void usage(const char *prog)
{
    fprintf(stderr, "Usage: %s [-n] file\n", prog);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    int wait_child = 1;
    int opt;
    pid_t pid;

    while ((opt = getopt(argc, argv, "n")) != -1) {
        if (opt == 'n')
            wait_child = 0;
        else
            usage(argv[0]);
    }
    if (optind >= argc)
        usage(argv[0]);

    printf("[PARENT] Program started, my PID: %ld\n", (long)getpid());

    /*
     * Flush stdout before fork(). Otherwise, when output goes to a file or
     * a pipe, the unflushed buffer is copied into the child and printed twice.
     */
    fflush(stdout);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        printf("[CHILD] I am the child process, my PID: %ld\n", (long)getpid());
        fflush(stdout);
        execlp("cat", "cat", argv[optind], (char *)NULL);
        /* We get here only if exec failed. */
        perror("execlp");
        _exit(127);
    }

    printf("[PARENT] I am the parent, child PID: %ld\n", (long)pid);
    printf("[PARENT] Printing some text while the child is working...\n");

    if (wait_child) {
        int status;

        printf("[PARENT] Waiting for the child to finish...\n");
        fflush(stdout);
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
        if (WIFEXITED(status))
            printf("[PARENT] Child exited with status %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("[PARENT] Child was killed by signal %d\n", WTERMSIG(status));
    }

    printf("[PARENT] This is the last line printed by the parent.\n");
    return 0;
}
