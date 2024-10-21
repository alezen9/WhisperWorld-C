#ifndef LIST_H
#define LIST_H

#include <time.h>

#define MAX_MSG_LENGTH 256
#define USER_NAME_LENGTH 32

struct Message {
    char user_name[USER_NAME_LENGTH];
    time_t timestamp;
    char content[MAX_MSG_LENGTH];
    struct Message *next;
};

struct List {
    struct Message *head;
    struct Message *tail;
};

// Function declarations
void list_append(struct Message *message, struct List *list);
void list_print(struct List *list);
void list_deallocate(struct List *list);

#endif // LIST_H
