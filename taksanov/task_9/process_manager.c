#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    // 1. Проверяем аргументы командной строки
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Starting program. PID: %ld\n", (long)getpid());

    // 2. Порождаем подпроцесс
    pid_t pid = fork();

    if (pid < 0) {
        perror("fork failed");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        // --- КОД ПОДПРОЦЕССА (CHILD) ---
        printf("[CHILD] Child started. PID: %ld, Parent PID: %ld\n", 
               (long)getpid(), (long)getppid());

        // Заменяем текущий процесс утилитой cat
        // Первый аргумент — имя программы, второй — argv[0], далее аргументы и NULL
        execlp("cat", "cat", argv[1], (char *)NULL);

        // Код ниже сработает ТОЛЬКО если execlp завершился с ошибкой
        perror("execlp failed");
        exit(EXIT_FAILURE);
    } else {
        // --- КОД РОДИТЕЛЯ (PARENT) ---
        printf("[PARENT] Child created with PID: %ld\n", (long)pid);
        printf("[PARENT] Printing some text while child is running...\n");

        int status;
        printf("[PARENT] Waiting for child to finish...\n");

        // Блокирующее ожидание завершения порожденного процесса
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid failed");
            return EXIT_FAILURE;
        }

        // Проверяем статус завершения
        if (WIFEXITED(status)) {
            printf("[PARENT] Child exited normally with code: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("[PARENT] Child terminated by signal: %d\n", WTERMSIG(status));
        }

        // Строка, которая гарантированно выводится строго ПОСЛЕ завершения потомка
        printf("[PARENT] Child has finished. This is the last line printed by parent.\n");
    }

    return EXIT_SUCCESS;
}

