#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 1024

// Структура узла связного списка
struct Node {
    char *data;         // Указатель на выделенную строку
    struct Node *next;  // Указатель на следующий узел
};

// Функция добавления новой строки в конец списка
void append(struct Node **head, struct Node **tail, const char *str) {
    // 1. Выделяем память под сам узел
    struct Node *new_node = (struct Node *)malloc(sizeof(struct Node));
    if (new_node == NULL) {
        perror("malloc node failed");
        exit(EXIT_FAILURE);
    }

    // 2. Выделяем память под строку (длина + 1 байт под символ '\0')
    new_node->data = (char *)malloc(strlen(str) + 1);
    if (new_node->data == NULL) {
        perror("malloc string failed");
        free(new_node);
        exit(EXIT_FAILURE);
    }

    // 3. Копируем текст строки и зануляем указатель next
    strcpy(new_node->data, str);
    new_node->next = NULL;

    // 4. Добавляем узел в конец списка (сохраняя порядок ввода)
    if (*head == NULL) {
        *head = new_node;
        *tail = new_node;
    } else {
        (*tail)->next = new_node;
        *tail = new_node;
    }
}

// Функция полного освобождения памяти списка
void free_list(struct Node *head) {
    struct Node *curr = head;
    while (curr != NULL) {
        struct Node *next = curr->next;
        free(curr->data); // Сначала освобождаем строку
        free(curr);       // Затем освобождаем сам узел
        curr = next;
    }
}

int main(void) {
    struct Node *head = NULL;
    struct Node *tail = NULL;
    char buffer[BUFFER_SIZE];

    printf("Enter strings (start line with '.' to stop):\n");

    // Читаем строки из стандартного ввода построчно
    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        // Убираем символ переноса строки '\n' в конце, если он есть
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
        }

        // Если первый символ точка — останавливаем ввод
        if (buffer[0] == '.') {
            break;
        }

        // Добавляем строку в список
        append(&head, &tail, buffer);
    }

    // Выводим все сохраненные строки
    printf("\n--- Result lines from list ---\n");
    struct Node *curr = head;
    while (curr != NULL) {
        printf("%s\n", curr->data);
        curr = curr->next;
    }

    // Обязательно освобождаем всю выделенную динамическую память
    free_list(head);

    return 0;
}
