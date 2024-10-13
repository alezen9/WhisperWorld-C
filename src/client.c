#include "list.h"
#include <arpa/inet.h>
#include <errno.h> // For handling error codes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERVER_PORT 8080

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

int connect_to_server() {
    int client_fd;
    struct sockaddr_in server_address;

    // Configure server address
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(SERVER_PORT);

    // Create socket
    if ((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        exit(EXIT_FAILURE); // Fail fast if socket creation fails
    }

    // Convert IP addresses from text to binary (localhost)
    if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(client_fd);   // Cleanup if conversion fails
        exit(EXIT_FAILURE); // Fail fast if address conversion fails
    }

    // Connect to the server
    if (connect(client_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Connection failed");
        close(client_fd);   // Cleanup if connection fails
        exit(EXIT_FAILURE); // Fail fast if connection fails
    }

    return client_fd;
}

void print_chat_title() {
    printf("*************************************************\n");
    printf("*                                               *\n");
    printf("*       Yokoso, watashi no Wispa Warudo!!       *\n");
    printf("*                                               *\n");
    printf("*************************************************\n");
}

void print_chat_log(struct List *list, char *user_name, const char *error_message) {
    printf(CLEAR_SCREEN);
    print_chat_title();
    printf("%s%sChat log%s\n", BOLD_TEXT, BLUE_TEXT, RESET_TEXT);

    list_print(list);
    printf("\n\n");

    if (error_message != NULL) printf("%s%s%s", RED_TEXT, error_message, RESET_TEXT);
    printf("[%s] Type your message (type 'quit' to exit): ", user_name);
    fflush(stdout); // Ensure the output is flushed to the console immediately
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

void set_user(char *user_name) {
    char name[sizeof(user_name)];
    // Ensure reading from stdin was successful
    if (fgets(name, sizeof(name), stdin) == NULL) {
        printf("Error reading input.\n");
        clear_input_buffer();
    }
    name[strcspn(name, "\n")] = 0; // Remove newline character
    strcpy(user_name, name);
}

void send_message(int client_fd, struct Message *message) {
    if (send(client_fd, message, sizeof(struct Message), 0) == -1) perror("Error sending message struct");
}

void receive_message(int server_fd, struct Message *msg) {
    int valread = recv(server_fd, msg, sizeof(struct Message), 0);
    if (valread <= 0) {
        if (valread == 0)
            printf("\nServer disconnected.\n"); // Server has closed the connection
        else
            perror("Error receiving message from server");
        close(server_fd);   // Close the socket to clean up
        exit(EXIT_FAILURE); // Exit if server disconnects or error occurs
    }
}

int main() {
    int client_fd = connect_to_server();
    char user_name[USER_NAME_LENGTH];
    struct List list = {NULL, NULL};
    char input[MAX_MSG_LENGTH];
    struct Message current_message;

    // Ask for user name
    printf(CLEAR_SCREEN);
    print_chat_title();
    printf("\nEnter your name: ");
    set_user(user_name);

    const char *error_message = NULL;

    print_chat_log(&list, user_name, error_message);

    fd_set fds; // Set of file descriptors to monitor
    while (strcmp(input, "quit") != 0) {
        FD_ZERO(&fds);
        FD_SET(STDIN_FILENO, &fds);
        FD_SET(client_fd, &fds);

        int activity = select(client_fd + 1, &fds, NULL, NULL, NULL);

        if (activity < 0 && errno != EINTR) {
            perror("Select error");
            close(client_fd);
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(STDIN_FILENO, &fds)) {
            error_message = NULL;

            if (fgets(input, sizeof(input), stdin) == NULL) {
                printf("Error reading input.\n");
                clear_input_buffer();
                continue;
            }

            if (strcmp(input, "quit\n") == 0) break;

            input[strcspn(input, "\n")] = 0; // Remove newline

            int is_valid = is_input_valid(input, &error_message);
            if (is_valid != INPUT_VALID) {
                if (is_valid == INPUT_ERROR_TOO_LONG) clear_input_buffer();
                print_chat_log(&list, user_name, error_message);
                continue;
            }

            strcpy(current_message.content, input);
            strcpy(current_message.user_name, user_name);
            current_message.timestamp = time(NULL);
            send_message(client_fd, &current_message);

            list_append(&current_message, &list);
            print_chat_log(&list, user_name, error_message);
        }

        if (FD_ISSET(client_fd, &fds)) {
            receive_message(client_fd, &current_message);
            list_append(&current_message, &list);
            print_chat_log(&list, user_name, error_message);
        }
    }
    printf("👋 Goodbye!\n");

    close(client_fd);
    return EXIT_SUCCESS;
}
