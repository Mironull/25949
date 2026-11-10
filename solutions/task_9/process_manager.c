/*
 * Task 9. Creating two processes.
 *
 * The program creates a child process that runs cat(1) on a long file.
 * Meanwhile the parent prints its own text with printf(3).
 *
 * The same source is built into several programs that differ only in how
 * the parent waits for the child. The variant is chosen at compile time
 * with -DWAIT_MODE=<mode> (see the Makefile):
 *
 *   MODE_NOWAIT     the parent does not wait at all (first part of the task):
 *                   its last line usually appears before or inside the
 *                   output of cat;
 *   MODE_WAITPID    waitpid(3C) before the last line (second part of the
 *                   task): the last line is always after the output of cat;
 *   MODE_WAIT       the same with wait(2), which waits for any child;
 *   MODE_WAITID     the same with waitid(2), which returns a siginfo_t
 *                   instead of a packed status word;
 *   MODE_WAIT_EARLY waitpid right after fork(): the parent prints nothing
 *                   until cat has finished;
 *   MODE_ZOMBIE     the parent does not wait but stays alive for a while
 *                   and shows with ps(1) that the finished child is a zombie.
 *
 * Usage: ./process_<variant> file
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MODE_NOWAIT      0
#define MODE_WAITPID     1
#define MODE_WAIT        2
#define MODE_WAITID      3
#define MODE_WAIT_EARLY  4
#define MODE_ZOMBIE      5

#ifndef WAIT_MODE
#define WAIT_MODE MODE_WAITPID
#endif

/*
 * Print a line and flush it at once. Without the flush, when the output goes
 * to a pipe or a file, the parent's text would stay in the stdio buffer until
 * exit and the real order of events would not be visible.
 */
#define SAY(...) do { printf(__VA_ARGS__); fflush(stdout); } while (0)

#if WAIT_MODE == MODE_WAITPID || WAIT_MODE == MODE_WAIT || WAIT_MODE == MODE_WAIT_EARLY
/* Decode the packed status word returned by wait(2) and waitpid(3C). */
static void report_status(int status)
{
    if (WIFEXITED(status))
        SAY("[PARENT] Child exited with status %d\n", WEXITSTATUS(status));
    else if (WIFSIGNALED(status))
        SAY("[PARENT] Child was killed by signal %d\n", WTERMSIG(status));
}

/* Wait for the given child with waitpid(3C) and print how it finished. */
static void wait_with_waitpid(pid_t pid)
{
    int status;

    SAY("[PARENT] Waiting for the child with waitpid()...\n");
    if (waitpid(pid, &status, 0) == -1) {
        perror("waitpid");
        exit(EXIT_FAILURE);
    }
    report_status(status);
}
#endif

int main(int argc, char **argv)
{
    pid_t pid;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return EXIT_FAILURE;
    }

    SAY("[PARENT] Program started, my PID: %ld\n", (long)getpid());

    /*
     * The stdio buffer must be empty before fork(): fork() copies the whole
     * memory of the process, so unflushed text would be printed twice.
     * SAY() has already flushed it; the call is kept here to make the rule
     * visible in the code.
     */
    fflush(stdout);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        SAY("[CHILD] I am the child process, my PID: %ld\n", (long)getpid());
        execlp("cat", "cat", argv[1], (char *)NULL);
        /* We get here only if exec failed. */
        perror("execlp");
        _exit(127);
    }

#if WAIT_MODE == MODE_WAIT_EARLY
    /* Wait before printing anything else: all parent text goes after cat. */
    wait_with_waitpid(pid);
#endif

    SAY("[PARENT] I am the parent, child PID: %ld\n", (long)pid);
    SAY("[PARENT] Printing some text while the child is working...\n");

#if WAIT_MODE == MODE_WAITPID
    wait_with_waitpid(pid);

#elif WAIT_MODE == MODE_WAIT
    {
        int status;
        pid_t done;

        /* wait() does not take a PID: it returns when any child finishes. */
        SAY("[PARENT] Waiting for any child with wait()...\n");
        done = wait(&status);
        if (done == -1) {
            perror("wait");
            return EXIT_FAILURE;
        }
        SAY("[PARENT] wait() returned PID %ld\n", (long)done);
        report_status(status);
    }

#elif WAIT_MODE == MODE_WAITID
    {
        siginfo_t info;

        /*
         * waitid() fills a siginfo_t: si_code tells how the child finished,
         * si_status holds the exit code or the signal number.
         */
        SAY("[PARENT] Waiting for the child with waitid()...\n");
        if (waitid(P_PID, pid, &info, WEXITED) == -1) {
            perror("waitid");
            return EXIT_FAILURE;
        }
        if (info.si_code == CLD_EXITED)
            SAY("[PARENT] Child %ld exited with status %d\n",
                (long)info.si_pid, info.si_status);
        else if (info.si_code == CLD_KILLED || info.si_code == CLD_DUMPED)
            SAY("[PARENT] Child %ld was killed by signal %d\n",
                (long)info.si_pid, info.si_status);
    }

#elif WAIT_MODE == MODE_ZOMBIE
    {
        char cmd[64];

        /*
         * Nobody calls wait(), so after cat exits the kernel keeps its entry
         * (PID and exit status) until the parent dies. ps shows it as Z.
         */
        sleep(2);
        SAY("[PARENT] The child has finished, but nobody called wait(). ps:\n");
        snprintf(cmd, sizeof(cmd), "ps -o pid,ppid,s,comm -p %ld", (long)pid);
        if (system(cmd) == -1)
            perror("system");
    }
#endif

    SAY("[PARENT] This is the last line printed by the parent.\n");
    return 0;
}
