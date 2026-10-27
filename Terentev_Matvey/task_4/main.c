#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_STR_LEN 1024

typedef struct node{
    char *str;
    struct node *next;
} Node;

typedef struct {
    Node *head;
    Node *tail;
} List;

void push(List *list, const char *str){
    Node *temp = (Node*)malloc(sizeof(Node));
    temp->str = (char*)malloc(strlen(str) + 1);
    temp->next = NULL;
    strcpy(temp->str, str);
    if (list->tail == NULL){
        list->tail = temp;
        list->head = temp;
        return;
    }
    list->tail->next = temp;
    list->tail = temp;
}

void print_list(List *list) {
    for (Node *cur = list->head; cur != NULL; cur = cur->next) {
        printf("%s", cur->str);
    }
}

int main() {
    List list;
    list.head = NULL;
    list.tail = NULL;
    char *buffer = (char*)malloc(sizeof(char) * (MAX_STR_LEN + 1));
    fgets(buffer, MAX_STR_LEN, stdin);
     while (buffer[0] != '.') {
        push(&list, buffer);
        fgets(buffer, MAX_STR_LEN, stdin);
    }
    print_list(&list);
    return 0;
}
