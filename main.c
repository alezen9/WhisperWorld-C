#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_MSG_LENGTH 255
#define MAX_VALID_LENGTH (MAX_MSG_LENGTH - 1)

#define INPUT_VALID 1
#define INPUT_ERROR_EMPTY 0
#define INPUT_ERROR_TOO_LONG -1

#define CLEAR_SCREEN "\033[2J\033[H" // Clear screen and move cursor to top
#define BOLD_TEXT "\033[1m"
#define RED_TEXT "\033[31m"
#define BLUE_TEXT "\033[34m"
#define PURPLE_TEXT "\033[35m"
#define RESET_TEXT "\033[0m"

void clear_input_buffer() {
  int c;
  while ((c = getchar()) != '\n' && c != EOF)
    ; // Discard all characters in input buffer
}

void print_chat_title() {
  printf("*************************************************\n");
  printf("*                                               *\n");
  printf("*       Yokoso, watashi no Wispa Warudo!!       *\n");
  printf("*                                               *\n");
  printf("*************************************************\n");
}

void print_chat_log(struct List *list, const char *error_message) {
  printf(CLEAR_SCREEN); // Clear the entire screen
  print_chat_title();
  printf("%s%sChat log%s\n", BOLD_TEXT, BLUE_TEXT, RESET_TEXT);

  // Print all previous messages in the chat log
  list_print(list);
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
  struct List list = {NULL, NULL};

  const char *error_message = NULL;

  while (strcmp(input, "quit") != 0) {
    print_chat_log(&list, error_message);

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
    list_append(input, &list);
  }
  print_chat_log(&list, error_message);
  printf("quit\n");
  printf("👋 Goodbye!\n");
  list_deallocate(&list);
  return 0;
}