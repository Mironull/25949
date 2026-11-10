/*
 * Tasks 2 and 3 combined into one program.
 *
 * The program prints the current time in California (task 2), then prints
 * the real and effective UID of the process before and after dropping
 * privileges with setuid(getuid()) (task 3).
 *
 * The time zone is set via TZ=America/Los_Angeles, so the switch between
 * PST and PDT is handled automatically.
 *
 * If the executable has the setuid bit and another user runs it, the
 * effective UID equals the file owner's UID before the drop and becomes
 * the real UID of the caller after the drop.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

static void print_california_time(void)
{
    time_t now;
    struct tm *sp;

    if (setenv("TZ", "America/Los_Angeles", 1) == -1) {
        perror("setenv");
        exit(EXIT_FAILURE);
    }
    tzset();

    time(&now);
    sp = localtime(&now);
    if (sp == NULL) {
        perror("localtime");
        exit(EXIT_FAILURE);
    }

    printf("California time: %02d/%02d/%04d %02d:%02d %s\n",
           sp->tm_mon + 1, sp->tm_mday, sp->tm_year + 1900,
           sp->tm_hour, sp->tm_min, tzname[sp->tm_isdst > 0 ? 1 : 0]);
}

static void print_uids(const char *stage)
{
    printf("%-24s real uid = %ld, effective uid = %ld\n",
           stage, (long)getuid(), (long)geteuid());
}

int main(void)
{
    print_california_time();

    print_uids("Before setuid(getuid()):");

    if (setuid(getuid()) == -1) {
        perror("setuid");
        exit(EXIT_FAILURE);
    }

    print_uids("After setuid(getuid()):");
    return 0;
}
