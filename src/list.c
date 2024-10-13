#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

void list_append(struct Message *message, struct List *list) {
    struct Message *current = (struct Message *)malloc(sizeof(struct Message));
    if (current == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    current->timestamp = message->timestamp;
    strncpy(current->content, message->content, MAX_MSG_LENGTH - 1);
    current->content[MAX_MSG_LENGTH - 1] = '\0'; // Ensure null termination
    current->next = NULL;
    strcpy(current->user_name, message->user_name);

    if (list->head == NULL)
        list->head = current;
    else
        list->tail->next = current;
    list->tail = current;
}

void list_print(struct List *list) {
    if (list->head == NULL) {
        printf("(empty)\n");
        return;
    }

    struct Message *current = list->head;
    while (current != NULL) {
        struct tm *time_info = localtime(&current->timestamp);
        char timeString[6]; // space for "HH:MM\0"
        strftime(timeString, sizeof(timeString), "%H:%M", time_info);
        printf("[%s, %s] %s\n", timeString, current->user_name, current->content);
        current = current->next;
    }
    printf("\n");
}

void list_deallocate(struct List *list) {
    while (list->head != NULL) {
        struct Message *next = list->head->next;
        free(list->head);
        list->head = next;
    }
    list->tail = NULL;
}
