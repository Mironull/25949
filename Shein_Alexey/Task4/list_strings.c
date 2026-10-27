#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct Node {
    char *data;
    struct Node *next;
};

static int append(struct Node **head, struct Node **tail,
                  const char *str)
{
    struct Node *node = malloc(sizeof(*node));

    if (node == NULL) {
        perror("malloc node");
        return -1;
    }

    node->data = malloc(strlen(str) + 1);
    if (node->data == NULL) {
        perror("malloc string");
        free(node);
        return -1;
    }

    strcpy(node->data, str);
    node->next = NULL;

    if (*tail == NULL) {
        *head = node;
    } else {
        (*tail)->next = node;
    }

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
    char buffer[1024];
    struct Node *head = NULL;
    struct Node *tail = NULL;
    struct Node *node;
    int status = EXIT_SUCCESS;

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        size_t length;
        int c;

        if (buffer[0] == '.') {
            break;
        }

        length = strlen(buffer);

        if (length > 0 && buffer[length - 1] == '\n') {
            buffer[length - 1] = '\0';
        } else {
            /* Check that fgets read the entire line. */
            c = getchar();
            if (c != '\n' && c != EOF) {
                fprintf(stderr,
                        "Input line exceeds 1023 bytes\n");
                status = EXIT_FAILURE;
                break;
            }
        }

        if (append(&head, &tail, buffer) == -1) {
            status = EXIT_FAILURE;
            break;
        }
    }

    if (ferror(stdin)) {
        perror("stdin");
        status = EXIT_FAILURE;
    }

    if (status == EXIT_SUCCESS) {
        for (node = head; node != NULL; node = node->next) {
            if (puts(node->data) == EOF) {
                perror("stdout");
                status = EXIT_FAILURE;
                break;
            }
        }

        if (fflush(stdout) == EOF) {
            perror("stdout");
            status = EXIT_FAILURE;
        }
    }

    free_list(head);
    return status;
}
