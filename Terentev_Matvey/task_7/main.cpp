#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <vector>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

struct LineInfo {
    off_t offset;
    size_t length;
};

static volatile sig_atomic_t g_timeout = 0;

static void on_alarm(int)
{
    g_timeout = 1;
}

static std::vector<LineInfo> build_table(const char* data, size_t size)
{
    std::vector<LineInfo> table;
    size_t line_start = 0;

    for (size_t i = 0; i < size; ++i) {
        if (data[i] == '\n') {
            table.push_back({ static_cast<off_t>(line_start), i - line_start });
            line_start = i + 1;
        }
    }

    if (size > line_start) {
        table.push_back({ static_cast<off_t>(line_start), size - line_start });
    }

    return table;
}

static void print_table(const std::vector<LineInfo>& table)
{
    std::printf("--- Line table ---\n");
    for (size_t i = 0; i < table.size(); ++i) {
        std::printf("line %zu: offset = %lld, length = %zu\n",
                    i + 1,
                    static_cast<long long>(table[i].offset),
                    table[i].length);
    }
    std::printf("-------------------\n");
}

static void print_line(const char* data, const std::vector<LineInfo>& table, size_t line_no)
{
    if (line_no < 1 || line_no > table.size()) {
        std::fprintf(stderr, "Line number out of range (1..%zu)\n", table.size());
        return;
    }

    const LineInfo& li = table[line_no - 1];
    std::fwrite(data + li.offset, 1, li.length, stdout);
    std::fputc('\n', stdout);
}

static void print_whole_file(const char* data, const std::vector<LineInfo>& table)
{
    std::printf("--- Timeout: printing whole file ---\n");
    for (size_t i = 0; i < table.size(); ++i)
        print_line(data, table, i + 1);
}

static bool read_line_with_timeout(char* buf, size_t bufsize, ssize_t* out_len)
{
    for (;;) {
        ssize_t r = read(STDIN_FILENO, buf, bufsize - 1);
        if (r == -1) {
            if (errno == EINTR) {
                if (g_timeout)
                    return false;
                continue;
            }
            std::perror("read(stdin)");
            return false;
        }
        if (r == 0) {
            *out_len = 0;
            return true;
        }
        buf[r] = '\0';
        *out_len = r;
        return true;
    }
}

int main(int argc, char* argv[])
{
    const char* path = (argc > 1) ? argv[1] : "text_file.txt";

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        std::perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        std::perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size == 0) {
        std::printf("File is empty\n");
        close(fd);
        return 0;
    }

    size_t size = static_cast<size_t>(st.st_size);

    void* raw = mmap(nullptr, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (raw == MAP_FAILED) {
        std::perror("mmap");
        close(fd);
        return 1;
    }

    const char* data = static_cast<const char*>(raw);

    close(fd);

    std::vector<LineInfo> table = build_table(data, size);
    print_table(table);

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, nullptr) == -1) {
        std::perror("sigaction");
        munmap(raw, size);
        return 1;
    }

    char inbuf[256];

    for (;;) {
        std::printf("Enter line number (0 to quit, 5 sec timeout): ");
        std::fflush(stdout);

        g_timeout = 0;
        alarm(5);

        ssize_t len = 0;
        bool ok = read_line_with_timeout(inbuf, sizeof(inbuf), &len);

        alarm(0);

        if (!ok || g_timeout) {
            print_whole_file(data, table);
            munmap(raw, size);
            return 0;
        }

        if (len == 0)
            break;

        char* end = nullptr;
        long n = std::strtol(inbuf, &end, 10);
        if (end == inbuf) {
            std::printf("Not a number, try again\n");
            continue;
        }
        if (n == 0)
            break;
        if (n < 0) {
            std::printf("Negative number, try again\n");
            continue;
        }

        print_line(data, table, static_cast<size_t>(n));
    }

    munmap(raw, size);
    return 0;
}
