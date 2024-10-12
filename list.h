#ifndef LIST_H
#define LIST_H

#include <time.h>

#define MAX_MSG_LENGTH 255

struct User {
  char name[50];
};

struct Message {
  struct User *user;
  time_t timestamp;
  char content[MAX_MSG_LENGTH];
  struct Message *next;
};

struct List {
  struct Message *head;
  struct Message *tail;
};

// Function declarations
void list_append(char *input, struct User **current_user, struct List *list);
void list_print(struct List *list);
void list_deallocate(struct List *list);

#endif // LIST_H
