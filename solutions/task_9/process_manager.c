/*
 * Задание 9. Создание двух процессов.
 *
 * Программа создаёт подпроцесс, который исполняет cat(1) для длинного файла.
 * Родитель тем временем печатает свой текст через printf(3).
 *
 * По умолчанию родитель ждёт завершения ребёнка через waitpid(2), поэтому его
 * последняя строка выводится строго после всего вывода cat (вторая часть
 * задания). С ключом -n родитель не ждёт ребёнка, и его последняя строка
 * может оказаться где угодно посреди вывода cat (первая часть задания).
 *
 * Запуск: ./process_manager [-n] файл
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    int wait_child = 1;
    int opt;
    pid_t pid;

    while ((opt = getopt(argc, argv, "n")) != -1) {
        if (opt == 'n') {
            wait_child = 0;
        } else {
            fprintf(stderr, "Использование: %s [-n] файл\n", argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (optind >= argc) {
        fprintf(stderr, "Использование: %s [-n] файл\n", argv[0]);
        return EXIT_FAILURE;
    }

    printf("[PARENT] Программа запущена, мой PID: %ld\n", (long)getpid());

    /*
     * Буфер stdout сбрасывается до fork(). Иначе при выводе в файл или канал
     * несброшенный текст скопируется в ребёнка и напечатается дважды.
     */
    fflush(stdout);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }

    if (pid == 0) {
        printf("[CHILD] Я дочерний процесс, мой PID: %ld\n", (long)getpid());
        fflush(stdout);
        execlp("cat", "cat", argv[optind], (char *)NULL);
        /* Сюда управление попадает, только если exec не удался. */
        perror("execlp");
        _exit(127);
    }

    printf("[PARENT] Я родитель, PID ребёнка: %ld\n", (long)pid);
    printf("[PARENT] Печатаю текст, пока ребёнок работает...\n");

    if (wait_child) {
        int status;

        printf("[PARENT] Жду завершения ребёнка...\n");
        fflush(stdout);
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            return EXIT_FAILURE;
        }
        if (WIFEXITED(status))
            printf("[PARENT] Ребёнок завершился с кодом %d\n", WEXITSTATUS(status));
        else if (WIFSIGNALED(status))
            printf("[PARENT] Ребёнок убит сигналом %d\n", WTERMSIG(status));
    }

    printf("[PARENT] Это последняя строка, которую печатает родитель.\n");
    return 0;
}
