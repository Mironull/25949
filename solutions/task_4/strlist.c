/* Задание 4. Список строк. Ввод завершается строкой, начинающейся с точки. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 1024

struct node {
    char *str;
    struct node *next;
};

int main(void)
{
    char buf[MAXLEN];
    struct node *head = NULL, *tail = NULL, *p, *next;
    size_t len;

    while (fgets(buf, sizeof(buf), stdin) != NULL && buf[0] != '.') {
        len = strlen(buf);

        p = malloc(sizeof(struct node));
        if (p == NULL || (p->str = malloc(len + 1)) == NULL) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(p->str, buf);
        p->next = NULL;

        if (head == NULL)
            head = p;
        else
            tail->next = p;
        tail = p;
    }

    for (p = head; p != NULL; p = next) {
        next = p->next;
        fputs(p->str, stdout);
        free(p->str);
        free(p);
    }
    return 0;
}
