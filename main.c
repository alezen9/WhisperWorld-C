#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#define MAX_MSG_LENGTH 255

struct Message {
    time_t timestamp;
    char content[MAX_MSG_LENGTH];
    struct Message* next;
};

void list_append(time_t timestamp, char* input, struct Message** head, struct Message** tail, int* current_count){
    struct Message* current = (struct Message*)malloc(sizeof (struct Message));
    if (current == NULL) {
        printf("Memory allocation failed!\n");
        return;
    }
    // Initialize the new node with the provided data
    current->timestamp = timestamp;
    strncpy(current->content, input, MAX_MSG_LENGTH - 1);
    current->content[MAX_MSG_LENGTH - 1] = '\0'; // Ensure null termination
    current->next = NULL;

    if (*head == NULL) {
        *head = current;
    } else {
        (*tail)->next = current;
    }
    *tail = current;
    *current_count += 1;
}

void move_cursor_up(int n) {
    printf("\033[%dA", n);  // ANSI escape code to move cursor up by 'n' lines
}

void clear_line() {
    printf("\033[K");  // ANSI escape code to clear the current line
}

void list_print(struct Message* head, int* total_messages){
    if(head == NULL) {
        printf("(empty)\n");
        *total_messages = 0;
        return;
    }

    // Move cursor up by the number of lines that were previously printed
    if (*total_messages > 0) {
        move_cursor_up(*total_messages + 2);
    }

    // Clear all lines that were previously printed
    for (int i = 0; i < *total_messages + 2; i++) {
        clear_line();
        printf("\n");
    }

    move_cursor_up(*total_messages + 2);  // Move back up after clearing

    int lines_printed = 0;
    struct Message* current = head;
    while(current != NULL){
        struct tm * time_info = localtime(&current->timestamp);
        char timeString[6];  // space for "HH:MM\0"
        strftime(timeString, sizeof(timeString), "%H:%M", time_info);
        printf("[%s] User: %s\n", timeString, current->content);
        current = current->next;
        lines_printed++;
    }
    // Update the number of printed lines for future prints
    printf("\n");
    *total_messages = lines_printed;
}

void list_deallocate(struct Message** head, struct Message** tail){
    while(*head != NULL){
        *tail = (*head)->next;
        free(*head);
        *head = *tail;
    }
}

int main(void) {
    char input[MAX_MSG_LENGTH];
    struct Message* head = NULL;
    struct Message* tail = NULL;
    int messages_printed = 0;
    
    printf("Welcome to the Chat App!\n");
    printf("Chat log\n");
    list_print(head, &messages_printed);

    while (strcmp(input, "quit") != 0) {
        printf("\nType your message (type 'quit' to exit): ");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline character
        if(strcmp(input, "quit") == 0) break;
        list_append(time(NULL), input, &head, &tail, &messages_printed);
        list_print(head, &messages_printed);
    }

    printf("Goodbye!\n");
    list_deallocate(&head, &tail);
    return 0;
}
