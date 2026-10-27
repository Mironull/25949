#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <csignal>
#include <vector>
#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

struct LineInfo {
    off_t offset;
    size_t length;
};

static volatile sig_atomic_t g_timeout = 0;

static void on_alarm(int)
{
    g_timeout = 1;
}

static std::vector<LineInfo> build_table(int fd)
{
    std::vector<LineInfo> table;
    constexpr size_t BUF_SIZE = 4096;
    char buf[BUF_SIZE];

    off_t line_start = 0;
    off_t pos = 0;
    ssize_t n;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; ++i) {
            if (buf[i] == '\n') {
                table.push_back({ line_start,
                                  static_cast<size_t>((pos + i) - line_start) });
                line_start = pos + i + 1;
            }
        }
        pos += n;
    }

    if (n == -1) {
        std::perror("read");
        std::exit(1);
    }

    if (pos > line_start) {
        table.push_back({ line_start,
                          static_cast<size_t>(pos - line_start) });
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

static void print_line(int fd, const std::vector<LineInfo>& table, size_t line_no)
{
    if (line_no < 1 || line_no > table.size()) {
        std::fprintf(stderr, "Line number out of range (1..%zu)\n", table.size());
        return;
    }

    const LineInfo& li = table[line_no - 1];

    if (lseek(fd, li.offset, SEEK_SET) == static_cast<off_t>(-1)) {
        std::perror("lseek");
        return;
    }

    std::string line;
    line.resize(li.length);

    size_t total = 0;
    while (total < li.length) {
        ssize_t r = read(fd, &line[total], li.length - total);
        if (r == -1) {
            if (errno == EINTR)
                continue;
            std::perror("read");
            return;
        }
        if (r == 0)
            break;
        total += static_cast<size_t>(r);
    }
    line.resize(total);

    std::printf("%s\n", line.c_str());
}

void print_whole_file(int fd, const std::vector<LineInfo>& table)
{
    std::printf("--- Timeout: printing whole file ---\n");
    for (size_t i = 0; i < table.size(); ++i) {
        print_line(fd, table, i + 1);
    }
}

bool read_line_with_timeout(char* buf, size_t bufsize, ssize_t* out_len)
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

    std::vector<LineInfo> table = build_table(fd);
    print_table(table);

    struct sigaction sa;
    std::memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, nullptr) == -1) {
        std::perror("sigaction");
        close(fd);
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
            print_whole_file(fd, table);
            close(fd);
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

        print_line(fd, table, static_cast<size_t>(n));
    }

    close(fd);
    return 0;
}
