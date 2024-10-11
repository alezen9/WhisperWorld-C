#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_MSG_LENGTH 255
#define MAX_VALID_LENGTH (MAX_MSG_LENGTH - 1)

#define INPUT_VALID 1
#define INPUT_ERROR_EMPTY 0
#define INPUT_ERROR_TOO_LONG -1

#define CLEAR_SCREEN "\033[2J\033[H" // Clear screen and move cursor to top
#define RED_TEXT "\033[31m"
#define RESET_TEXT "\033[0m"

struct Message {
  time_t timestamp;
  char content[MAX_MSG_LENGTH];
  struct Message *next;
};

void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ; // Discard all characters in input buffer
}

void list_append(time_t timestamp, char *input, struct Message **head, struct Message **tail) {
  struct Message *current = (struct Message *)malloc(sizeof(struct Message));
  if (current == NULL) {
    printf("Memory allocation failed!\n");
    return;
  }
  // Initialize the new node with the provided data
  current->timestamp = timestamp;
  strncpy(current->content, input, MAX_VALID_LENGTH);
  current->content[MAX_VALID_LENGTH] = '\0'; // Ensure null termination
  current->next = NULL;

  if (*head == NULL) {
    *head = current;
  } else {
    (*tail)->next = current;
  }
  *tail = current;
}

void list_print(struct Message *head) {
  if (head == NULL) {
    printf("(empty)\n");
    return;
  }

  struct Message *current = head;
  while (current != NULL) {
    struct tm *time_info = localtime(&current->timestamp);
    char timeString[6]; // space for "HH:MM\0"
    strftime(timeString, sizeof(timeString), "%H:%M", time_info);
    printf("[%s] User: %s\n", timeString, current->content);
    current = current->next;
  }
  printf("\n");
}

void list_deallocate(struct Message **head) {
  while (*head != NULL) {
    struct Message *next = (*head)->next;
    free(*head);
    *head = next;
  }
}

// Function to print the chat log and current prompt
void print_chat_log(struct Message *head, const char *error_message) {
  printf(CLEAR_SCREEN); // Clear the entire screen
  printf("Yokoso, watashi no Wispa Warudo!!\n");
  printf("Chat log\n");

  // Print all previous messages in the chat log
  list_print(head);
  printf("\n\n");

  // If there's an error message, display it
  if (error_message != NULL) {
    printf("%s%s%s", RED_TEXT, error_message, RESET_TEXT);
  }

  // Print the input prompt
  printf("Type your message (type 'quit' to exit): ");
}

int is_input_valid(char *input, const char **error_message) {
  int len = strlen(input);

  if (len >= MAX_VALID_LENGTH) {
    *error_message = "Input cannot be more than 254 characters!\n";
    return INPUT_ERROR_TOO_LONG;
  }
  if (len == 0 || (len == 1 && input[0] == '\n')) {
    *error_message = "Input cannot be empty!\n";
    return INPUT_ERROR_EMPTY;
  }
  *error_message = NULL;
  return INPUT_VALID;
}

int main(void) {
  char input[MAX_MSG_LENGTH];
  struct Message *head = NULL;
  struct Message *tail = NULL;

  const char *error_message = NULL;

  while (strcmp(input, "quit") != 0) {
    print_chat_log(head, error_message);

    // Reset error message for the next loop
    error_message = NULL;

    // Ensure reading from stdin was successful
    if (fgets(input, sizeof(input), stdin) == NULL) {
      printf("Error reading input.\n");
      continue;
    }

    // Handle quit case
    if (strcmp(input, "quit\n") == 0)
      break;

    // Remove newline character
    input[strcspn(input, "\n")] = 0;

    // Validate input
    int is_valid = is_input_valid(input, &error_message);
    if (is_valid != INPUT_VALID) {
      if (is_valid == INPUT_ERROR_TOO_LONG)
        clear_input_buffer(); // Flush any extra characters left in stdin
      continue;
    }

    // Append valid input to the list and print
    list_append(time(NULL), input, &head, &tail);
  }
  print_chat_log(head, error_message);
  printf("quit\n");
  printf("Goodbye!\n");
  list_deallocate(&head);
  return 0;
}
