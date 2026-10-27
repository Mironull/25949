#include <stdio.h>        // printf, fgets, stdin
#include <stdlib.h>       // malloc, free, exit
#include <string.h>       // strlen, strcpy

struct Node
{
    char* data;
    struct Node *next;
};

void append(struct Node**head, char* string)
{
    struct Node *n = malloc(sizeof(struct Node));
    if (n == NULL) return;
    n->data = malloc(strlen(string)+1);
    if (n->data == NULL) {
        free(n);
        return;
    }
    strcpy(n->data, string);
    n->next = NULL; //новый узел всегда последний
    if (*head == NULL)
    {
        *head = n;
    } else
    {
        struct Node *cur = *head;
        while (cur->next != NULL) cur = cur->next;
        cur->next = n;

    }
}

void free_list(struct Node *head)
{
    while (head != NULL)
    {
        struct Node *next = head->next;
        free(head->data);
        free(head);
        head = next;
    }
}
int main()
{
    struct Node *head = NULL;
    char buffer[1024];
    while(1)
    {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL) break;
        size_t len = strlen(buffer);
        if (len > 0 && buffer[len-1] == '\n') buffer[len-1] = '\0';

        if (buffer[0] == '.') break;

        if (strlen(buffer) > 0) append(&head, buffer);
    }


    for (struct Node *node = head; node!=NULL; node = node->next)
    {
        printf("%s\n", node->data);
    }

    free_list(head);
    

    return 0;
}
