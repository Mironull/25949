#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    char *data;
    struct Node *next;
} Node;

void append(Node **head, const char *str) {
    Node *new_node = (Node *)malloc(sizeof(Node));
    if (new_node == NULL) {
        perror("malloc node");
        exit(EXIT_FAILURE);
    }

    new_node->data = (char *)malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("malloc data");
        free(new_node);
        exit(EXIT_FAILURE);
    }
    strcpy(new_node->data, str);
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        Node *curr = *head;
        while (curr->next != NULL) {
            curr = curr->next;
        }
        curr->next = new_node;
    }
}

void free_list(Node *head) {
    Node *curr = head;
    while (curr != NULL) {
        Node *temp = curr;
        curr = curr->next;
        free(temp->data);
        free(temp);
    }
}

int main(void) {
    Node *head = NULL;
    char buffer[1024];

    printf("Введите строки (точка '.' для завершения):\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == '.') {
            break;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        append(&head, buffer);
    }

    printf("\n=== Результат ===\n");
    if (head != NULL) {
        if (head->next == NULL) {
            // Граничный случай: всего 1 строка в списке
            printf("Единственная строка: %s\n", head->data);
        } else {
            // Ищем последний узел списка
            Node *tail = head;
            while (tail->next != NULL) {
                tail = tail->next;
            }

            // Выводим: сначала последняя, затем первая
            printf("Последняя: %s\n", tail->data);
            printf("Первая: %s\n", head->data);
        }
    } else {
        printf("Список пуст.\n");
    }

    free_list(head);

    return 0;
}
