/*
 * Task 9. Creating two processes.
 *
 * The program creates a child process that runs cat(1) on a long file.
 * Meanwhile the parent prints its own text with printf(3).
 *
 * The first argument selects how the parent waits for the child:
 *
 *   nowait   the parent does not wait at all (first part of the task):
 *            its last line usually appears before or inside the output of cat;
 *   waitpid  waitpid(3C) before the last line (second part of the task):
 *            the last line is always after the output of cat;
 *   wait     the same with wait(2), which waits for any child;
 *   waitid   the same with waitid(2), which returns a siginfo_t instead of
 *            a packed status word;
 *   early    waitpid right after fork(): the parent prints nothing until cat
 *            has finished;
 *   zombie   the parent does not wait but stays alive for a while and shows
 *            with ps(1) that the finished child is a zombie.
 *
 * Usage: ./process_manager mode file
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

enum mode { MODE_NOWAIT, MODE_WAITPID, MODE_WAIT, MODE_WAITID, MODE_EARLY, MODE_ZOMBIE };

static const struct {
    const char *name;
    enum mode mode;
} modes[] = {
    { "nowait",  MODE_NOWAIT  },
    { "waitpid", MODE_WAITPID },
    { "wait",    MODE_WAIT    },
    { "waitid",  MODE_WAITID  },
    { "early",   MODE_EARLY   },
    { "zombie",  MODE_ZOMBIE  },
};

#define NMODES (sizeof(modes) / sizeof(modes[0]))

/*
 * Print a line and flush it at once. Without the flush, when the output goes
 * to a pipe or a file, the parent's text would stay in the stdio buffer until
 * exit and the real order of events would not be visible.
 */
#define SAY(...) do { printf(__VA_ARGS__); fflush(stdout); } while (0)

static void usage(const char *prog)
{
    size_t i;

    fprintf(stderr, "Usage: %s mode file\nModes:", prog);
    for (i = 0; i < NMODES; i++)
        fprintf(stderr, " %s", modes[i].name);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

/* Turn the mode name from the command line into enum mode. */
static enum mode parse_mode(const char *name, const char *prog)
{
    size_t i;

    for (i = 0; i < NMODES; i++)
        if (strcmp(name, modes[i].name) == 0)
            return modes[i].mode;
    fprintf(stderr, "%s: unknown mode '%s'\n", prog, name);
    usage(prog);
    return MODE_WAITPID;   /* not reached: usage() exits */
}

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

/* wait() does not take a PID: it returns when any child finishes. */
static void wait_with_wait(void)
{
    int status;
    pid_t done;

    SAY("[PARENT] Waiting for any child with wait()...\n");
    done = wait(&status);
    if (done == -1) {
        perror("wait");
        exit(EXIT_FAILURE);
    }
    SAY("[PARENT] wait() returned PID %ld\n", (long)done);
    report_status(status);
}

/*
 * waitid() fills a siginfo_t: si_code tells how the child finished,
 * si_status holds the exit code or the signal number.
 */
static void wait_with_waitid(pid_t pid)
{
    siginfo_t info;

    SAY("[PARENT] Waiting for the child with waitid()...\n");
    if (waitid(P_PID, pid, &info, WEXITED) == -1) {
        perror("waitid");
        exit(EXIT_FAILURE);
    }
    if (info.si_code == CLD_EXITED)
        SAY("[PARENT] Child %ld exited with status %d\n",
            (long)info.si_pid, info.si_status);
    else if (info.si_code == CLD_KILLED || info.si_code == CLD_DUMPED)
        SAY("[PARENT] Child %ld was killed by signal %d\n",
            (long)info.si_pid, info.si_status);
}

/*
 * Nobody calls wait(), so after cat exits the kernel keeps its entry
 * (PID and exit status) until the parent dies. ps shows it as Z.
 */
static void show_zombie(pid_t pid)
{
    char cmd[64];

    sleep(2);
    SAY("[PARENT] The child has finished, but nobody called wait(). ps:\n");
    snprintf(cmd, sizeof(cmd), "ps -o pid,ppid,s,comm -p %ld", (long)pid);
    if (system(cmd) == -1)
        perror("system");
}

int main(int argc, char **argv)
{
    enum mode mode;
    pid_t pid;

    if (argc != 3)
        usage(argv[0]);
    mode = parse_mode(argv[1], argv[0]);

    SAY("[PARENT] Program started in mode '%s', my PID: %ld\n",
        argv[1], (long)getpid());

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
        execlp("cat", "cat", argv[2], (char *)NULL);
        /* We get here only if exec failed. */
        perror("execlp");
        _exit(127);
    }

    /* Wait before printing anything else: all parent text goes after cat. */
    if (mode == MODE_EARLY)
        wait_with_waitpid(pid);

    SAY("[PARENT] I am the parent, child PID: %ld\n", (long)pid);
    SAY("[PARENT] Printing some text while the child is working...\n");

    switch (mode) {
    case MODE_WAITPID:
        wait_with_waitpid(pid);
        break;
    case MODE_WAIT:
        wait_with_wait();
        break;
    case MODE_WAITID:
        wait_with_waitid(pid);
        break;
    case MODE_ZOMBIE:
        show_zombie(pid);
        break;
    case MODE_NOWAIT:
    case MODE_EARLY:
        break;
    }

    SAY("[PARENT] This is the last line printed by the parent.\n");
    return 0;
}
