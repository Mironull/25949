#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cerrno>
#include <vector>
#include <string>

#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

struct LineInfo {
    off_t offset;
    size_t length;
};

std::vector<LineInfo> build_table(int fd)
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

void print_table(const std::vector<LineInfo>& table)
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

void print_line(int fd, const std::vector<LineInfo>& table, size_t line_no)
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

    for (;;) {
        std::printf("Enter line number (0 to quit): ");
        std::fflush(stdout);

        char inbuf[64];
        if (std::fgets(inbuf, sizeof(inbuf), stdin) == nullptr)
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
