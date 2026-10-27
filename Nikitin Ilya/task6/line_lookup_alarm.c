#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

typedef struct
{
    off_t offset;
    off_t length;
} LineInfo;

static int fd;
static char *filename;

void alarm_handler(int signal_number)
{
    char buffer[1024];
    ssize_t bytes_read;
    (void)signal_number;
    int alarm_fd = open(filename, O_RDONLY);

    if (alarm_fd == -1)
    {
        const char message[] = "\n[Timeout] Cannot open file\n";
        write(STDOUT_FILENO, message, sizeof(message) - 1);
        _exit(EXIT_FAILURE);
    }

    {
        const char message[] = "\n[TIMEOUT] Time is up! Printing full file content...\n";
        write(STDOUT_FILENO, message, sizeof(message) - 1);
    }
    while ((bytes_read = read(alarm_fd, buffer, sizeof(buffer))) > 0)
    {
        write(STDOUT_FILENO, buffer, bytes_read);
    }
    close(alarm_fd);
    _exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    char ch;

    LineInfo *table = NULL;
    size_t num_lines = 0;
    size_t capacity = 0;
    off_t line_start = 0;
    off_t line_length = 0;

    if (argc != 2)
    {
        fprintf(stderr, "Usage %s <file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    filename = argv[1];
    fd = open(filename, O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return EXIT_FAILURE;
    }

    while (read(fd, &ch, 1) == 1)
    {
        if (ch == '\n')
        {
            if (num_lines == capacity)
            {
                size_t new_capacity;
                if (capacity == 0)
                    new_capacity = 10;
                else
                    new_capacity = capacity * 2;
                LineInfo *new_table = realloc(table, new_capacity * sizeof(LineInfo));
                if (new_table == NULL)
                {
                    fprintf(stderr, "Memory allocation failed\n");
                    free(table);
                    close(fd);
                    return EXIT_FAILURE;
                }
                table = new_table;
                capacity = new_capacity;
            }
            table[num_lines].offset = line_start;
            table[num_lines].length = line_length;
            num_lines++;

            line_start = lseek(fd, 0L, SEEK_CUR);
            line_length = 0;
        }
        else
        {
            line_length++;
        }
    }
    if (line_length > 0)
    {
        if (num_lines == capacity)
        {
            size_t new_capacity;
            if (capacity == 0)
                new_capacity = 10;
            else
                new_capacity = capacity * 2;
            LineInfo *new_table = realloc(table, new_capacity * sizeof(LineInfo));
            if (new_table == NULL)
            {
                fprintf(stderr, "Memory allocation failed\n");
                free(table);
                close(fd);
                return EXIT_FAILURE;
            }
            table = new_table;
            capacity = new_capacity;
        }
        table[num_lines].offset = line_start;
        table[num_lines].length = line_length;
        num_lines++;
    }

    printf("\n--- Line Table ---\n");
    for (size_t i = 0; i < num_lines; i++)
    {
        printf("Line %zu: offset = %ld, Length = %ld\n", i + 1, (long)table[i].offset, (long)table[i].length);
    }
    printf("------------------\n");

    if (signal(SIGALRM, alarm_handler) == SIG_ERR)
    {
        perror("signal");
        free(table);
        close(fd);
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
        if (lseek(fd, line.offset, SEEK_SET) == (off_t)-1)
        {
            perror("lseek");
            break;
        }

        char *buffer = malloc(line.length + 1);
        if (buffer == NULL)
        {
            fprintf(stderr, "Memore allocation failed\n");
            break;
        }

        ssize_t bytes_read = read(fd, buffer, line.length);
        if (bytes_read == -1)
        {
            perror("read");
            free(buffer);
            break;
        }
        buffer[bytes_read] = '\0';
        printf("%s\n", buffer);
        free(buffer);
    }

    free(table);
    if (close(fd) == -1)
    {
        perror("close");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
