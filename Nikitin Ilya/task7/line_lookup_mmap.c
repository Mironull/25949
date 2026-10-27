#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct
{
    off_t offset;
    off_t length;
} LineInfo;

static char *global_map = NULL;
static off_t global_size = 0;

void alarm_handler(int signal_number)
{
    (void)signal_number;
    {
        const char message[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
        if (write(STDOUT_FILENO, message, sizeof(message) - 1) == -1)
        {
            _exit(EXIT_FAILURE);
        }
    }
    if (write(STDOUT_FILENO, global_map, global_size) == -1)
    {
        _exit(EXIT_FAILURE);
    }

    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    int fd;
    struct stat st;

    LineInfo *table = NULL;
    size_t num_lines = 0;
    size_t capacity = 0;
    off_t line_start = 0;

    if (argc != 2)
    {
        fprintf(stderr, "Usage %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }

    global_size = st.st_size;
    if (global_size == 0)
    {
        printf("File is empty.\n");
        close(fd);
        return EXIT_SUCCESS;
    }

    global_map = mmap(NULL, global_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (global_map == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }

    if (close(fd) == -1)
    {
        perror("close");
        munmap(global_map, global_size);
        return EXIT_FAILURE;
    }

    for (off_t i = 0; i < global_size; i++)
    {
        if (global_map[i] == '\n')
        {
            if (num_lines == capacity)
            {
                size_t new_capacity;

                if (capacity == 0)
                {
                    new_capacity = 16;
                }
                else
                {
                    new_capacity = capacity * 2;
                }

                LineInfo *new_table = realloc(table, new_capacity * sizeof(LineInfo));

                if (new_table == NULL)
                {
                    fprintf(stderr, "Memory allocation failed\n");
                    free(table);
                    munmap(global_map, global_size);
                    return EXIT_FAILURE;
                }

                table = new_table;
                capacity = new_capacity;
            }

            table[num_lines].offset = line_start;
            table[num_lines].length = i - line_start;

            num_lines++;

            line_start = i + 1;
        }
    }
    if (line_start < global_size)
    {
        if (num_lines == capacity)
        {
            size_t new_capacity;

            if (capacity == 0)
            {
                new_capacity = 16;
            }
            else
            {
                new_capacity = capacity * 2;
            }

            LineInfo *new_table = realloc(table, new_capacity * sizeof(LineInfo));

            if (new_table == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                free(table);
                munmap(global_map, global_size);
                return EXIT_FAILURE;
            }

            table = new_table;
            capacity = new_capacity;
        }

        table[num_lines].offset = line_start;
        table[num_lines].length = global_size - line_start;

        num_lines++;
    }

    printf("--- Debug: Line Table ---\n");
    for (size_t i = 0; i < num_lines; i++)
    {
        printf("Line %zu: Offset=%lld, Length = %lld\n", i + 1, (long long)table[i].offset, (long long)table[i].length);
    }
    printf("--------------------------------\n");

    if (signal(SIGALRM, alarm_handler) == SIG_ERR)
    {
        perror("signal");
        free(table);
        munmap(global_map, global_size);
        return EXIT_FAILURE;
    }

    while (1)
    {
        long line_number;
        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        // ALARM
        alarm(5);

        if (scanf("%ld", &line_number) != 1)
        {
            alarm(0);
            fprintf(stderr, "Invalid input\n");
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF)
            {
            }
            continue;
        }

        alarm(0);
        if (line_number == 0)
            break;
        if (line_number < 1 || line_number > (long)num_lines)
        {
            printf("Invalid line number\n");
            continue;
        }

        LineInfo line = table[line_number - 1];
        if (fwrite(global_map + line.offset, 1, line.length, stdout) != (size_t)line.length)
        {
            fprintf(stderr, "Output error\n");
            break;
        }
        printf("\n");
    }

    free(table);
    if (munmap(global_map, global_size) == -1)
    {
        perror("munmap");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
