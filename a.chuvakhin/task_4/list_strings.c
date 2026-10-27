#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 1024

struct Node {
    char *data;
    struct Node *next;
};

static int append(struct Node **head, struct Node **tail, const char *str)
{
    struct Node *node = malloc(sizeof(struct Node));
    if (node == NULL)
        return -1;

    node->data = malloc(strlen(str) + 1); 
    if (node->data == NULL) {
        free(node);
        return -1;
    }
    strcpy(node->data, str);
    node->next = NULL;

    if (*head == NULL)
        *head = node;
    else
        (*tail)->next = node;
    *tail = node;
    return 0;
}

static void free_list(struct Node *head)
{
    while (head != NULL) {
        struct Node *next = head->next;
        free(head->data);
        free(head);
        head = next;
    }
}

int main(void)
{
    char buffer[BUF_SIZE];
    struct Node *head = NULL, *tail = NULL, *cur;

    printf("Enter lines (a line starting with '.' ends the input):\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == '.')
            break;

        buffer[strcspn(buffer, "\n")] = '\0';  

        if (append(&head, &tail, buffer) != 0) {
            fprintf(stderr, "Error: out of memory\n");
            free_list(head);
            return 1;
        }
    }

    printf("--- List ---\n");
    for (cur = head; cur != NULL; cur = cur->next)
        printf("%s\n", cur->data);

    free_list(head);
    return 0;
}
