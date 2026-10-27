#include <stdio.h>     // printf, fprintf
#include <stdlib.h>    // exit, EXIT_FAILURE
#include <unistd.h>    // fork, execvp
#include <sys/types.h> // pid_t
#include <sys/wait.h>  // waitpid, WIFEXITED, WEXITSTATUS
#include <string.h>    // strsignal

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s command [arguments...]\n", argv[0]);
        return EXIT_FAILURE;
    }
    printf("[PARENT] Starting command: %s\n", argv[1]);
    fflush(stdout);
    pid_t pid = fork();

    if (pid < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0)
    {
        printf("[Child] starting process\n");
        fflush(stdout);
        execvp(argv[1], &argv[1]);
        perror("execvp");
        exit(127);
    }

    int status;
    if (waitpid(pid, &status, 0) < 0)
    {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status))
    {
        printf("[Parent] Command exit normally with code: %d\n", WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        printf("[Parent] Command terninated by signal: %d (%s)\n", WTERMSIG(status), strsignal(WTERMSIG(status)));
    }

    return EXIT_SUCCESS;
}
