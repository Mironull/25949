#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    pid_t pid;
    int status;

    if (argc != 2)
    {
        fprintf(stderr, "Usage %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] STarting program. My PID: %d\n", (int)getpid());
    fflush(stdout);

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0)
    {
        printf("[CHILD] I am the child process. My PID: %d\n", (int)getpid());
        fflush(stdout);
        execlp("cat", "cat", argv[1], NULL);
        perror("execlp");
        exit(EXIT_FAILURE);
    }

    printf("[PARENT] I am the parent. Child PID: %d\n", (int)pid);
    printf("[PARENT] Printing some text while child is working...\n");
    fflush(stdout);

    if (waitpid(pid, &status, 0) == -1)
    {
        perror("waitpid");
        return EXIT_FAILURE;
    }

    if (WIFEXITED(status))
    {
        printf("[PARENT] child exited with status: %d\n", WEXITSTATUS(status));
    }
    else
    {
        printf("[PARENT] child did not exit normally.\n");
    }

    printf("[PARENT] Child has finished. This is the last line printed by parent.\n");

    return EXIT_SUCCESS;
}