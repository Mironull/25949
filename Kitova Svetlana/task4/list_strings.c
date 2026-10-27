#include <stdlib.h>
#include <string.h>
#include <stdio.h>


typedef struct Node {
    char *data;
    struct Node *next;
} Node;

void append(Node **head, const char *str) {
    Node *new_node = malloc(sizeof(struct Node));
    new_node->data = malloc(sizeof(strlen(str) + 1));

    if (new_node == NULL) {
        perror("malloc");
    } 
    if (new_node->data == NULL) {
        perror("malloc");
        free(new_node);
        exit(EXIT_FAILURE);
    }

    strcpy(new_node->data, str);
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
        return;
    }
    Node *current = *head;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_node;
}

void free_list(Node *head) {
    Node *current = head;
    while (current != NULL) {
        Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
}

int main(void) {
    char buffer[1024];
    Node *head = NULL;
    while(fgets(buffer, sizeof(buffer), stdin) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';
        if (buffer[0] == '.') {
            break;
        }
        append(&head, buffer);
    }
    printf("\nList:\n");
    Node *current = head;
    while (current != NULL) {
        printf("%s\n", current->data);
        current = current->next;
    }

    free_list(head);
    return EXIT_SUCCESS;
}