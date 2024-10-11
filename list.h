#ifndef LIST_H
#define LIST_H

#include <time.h>

#define MAX_MSG_LENGTH 255

// Structure for a message in the chat log
struct Message {
  time_t timestamp;
  char content[MAX_MSG_LENGTH];
  struct Message *next;
};

// Structure for the list (holds head and tail)
struct List {
  struct Message *head;
  struct Message *tail;
};

// Function declarations
void list_append(char *input, struct List *list);
void list_print(struct List *list);
void list_deallocate(struct List *list);

#endif // LIST_H
