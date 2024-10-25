// Includes
#include "list.h"
#include "serialization.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uv.h>

#define SERVER_PORT 8080
// Setting for uv_pipe_init to disable Inter-Process Communication (IPC).
// IPC allows data to be shared between processes; setting this to 0 means the pipe is only used for communication
// within this process, specifically for reading user input.
#define NO_IPC 0
#define MAX_VALID_LENGTH (MAX_MSG_LENGTH - 1)
#define INPUT_VALID 1
#define INPUT_ERROR_EMPTY 0
#define INPUT_ERROR_TOO_LONG -1
#define CLEAR_SCREEN "\033[2J\033[H"
#define BOLD_TEXT "\033[1m"
#define RED_TEXT "\033[31m"
#define BLUE_TEXT "\033[34m"
#define PURPLE_TEXT "\033[35m"
#define RESET_TEXT "\033[0m"

// Global Variables
uv_loop_t *loop;
uv_pipe_t stdin_pipe;
uv_tcp_t client_handle;
struct List chat_log = {NULL, NULL};
char user_name[USER_NAME_LENGTH];

// Cleanup Function
void cleanup_and_exit() {
    // Check if stdin listener is active and not closing, then close it
    if (uv_is_active((uv_handle_t *)&stdin_pipe) && !uv_is_closing((uv_handle_t *)&stdin_pipe))
        uv_close((uv_handle_t *)&stdin_pipe, NULL);

    // Check if server connection is active and not closing, then close it
    if (uv_is_active((uv_handle_t *)&client_handle) && !uv_is_closing((uv_handle_t *)&client_handle))
        uv_close((uv_handle_t *)&client_handle, NULL);

    // Free any dynamically allocated memory associated with message handling
    if (chat_log.head != NULL) list_deallocate(&chat_log); // Example if you’re using a linked list

    printf("👋 Goodbye!\n");
    uv_stop(loop);      // Ensure event loop stops
    exit(EXIT_SUCCESS); // Safely exit
}

// Signal Handler for Ctrl+C
void handle_sigint(uv_signal_t *req, int signum) {
    printf("\n\nCaught signal %d, exiting...\n", signum);
    cleanup_and_exit();
}

// Buffer Allocation
void allocate_read_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = malloc(suggested_size);
    buf->len = suggested_size;
}

// Display Chat Title and Prompt
void print_chat_title() {
    printf("*************************************************\n");
    printf("*                                               *\n");
    printf("*       Yokoso, watashi no Wispa Warudo!!       *\n");
    printf("*                                               *\n");
    printf("*************************************************\n");
}

// Display Chat Log
void display_chat_log(const char *error_message) {
    printf(CLEAR_SCREEN);
    print_chat_title();
    printf("%s%sChat log%s\n", BOLD_TEXT, BLUE_TEXT, RESET_TEXT);
    list_print(&chat_log);
    printf("\n\n");
    if (error_message != NULL) printf("%s%s%s", RED_TEXT, error_message, RESET_TEXT);
    printf("[%s] Type your message (type 'quit' to exit): ", user_name);
    fflush(stdout);
}

// Input Validation
int is_input_valid(char *input, const char **error_message) {
    int len = strlen(input);
    if (len >= MAX_VALID_LENGTH) {
        char error_buffer[64];
        snprintf(error_buffer, sizeof(error_buffer), "Input cannot be more than %d characters!\n", MAX_VALID_LENGTH);
        *error_message = error_buffer;
        return INPUT_ERROR_TOO_LONG;
    }
    if (len == 0 || (len == 1 && input[0] == '\n')) {
        *error_message = "Input cannot be empty!\n";
        return INPUT_ERROR_EMPTY;
    }
    *error_message = NULL;
    return INPUT_VALID;
}

// User Input Callback
void on_user_input(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    const char *error_message = NULL;
    if (nread <= 0) {
        if (nread < 0) fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
        free(buf->base);
        cleanup_and_exit();
        return;
    }

    // Remove newline and terminate
    buf->base[strcspn(buf->base, "\n")] = '\0';

    if (strcmp(buf->base, "quit") == 0) {
        free(buf->base);
        cleanup_and_exit();
        return;
    }

    int is_valid = is_input_valid(buf->base, &error_message);
    if (is_valid == INPUT_VALID) {
        struct Message msg;
        strcpy(msg.user_name, user_name);
        strcpy(msg.content, buf->base);
        msg.timestamp = time(NULL);

        char buffer[sizeof(struct Message)];
        if (serialize_message(&msg, buffer, sizeof(buffer)) < 0) {
            fprintf(stderr, "Serialization error\n");
        } else {
            uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t));
            uv_buf_t uvbuf = uv_buf_init(buffer, sizeof(buffer));
            uv_write(req, (uv_stream_t *)&client_handle, &uvbuf, 1, NULL);

            list_append(&msg, &chat_log);
            display_chat_log(error_message);
        }
    } else {
        display_chat_log(error_message);
    }

    free(buf->base);
}

// Server Message Callback
void on_server_message(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread > 0) {
        struct Message msg;
        if (deserialize_message(buf->base, &msg, nread) == 0) {
            list_append(&msg, &chat_log);
            display_chat_log(NULL);
        } else {
            fprintf(stderr, "Deserialization error\n");
        }
    } else if (nread < 0) {
        fprintf(stderr, "Server connection closed or read error: %s\n", uv_strerror(nread));
        free(buf->base);
        cleanup_and_exit();
    }

    free(buf->base);
}

// Connect to Server
void connect_to_server(uv_connect_t *connection_req) {
    struct sockaddr_in server_addr;
    uv_ip4_addr("127.0.0.1", SERVER_PORT, &server_addr);

    uv_tcp_init(loop, &client_handle);
    uv_tcp_connect(connection_req, &client_handle, (const struct sockaddr *)&server_addr, NULL);
    uv_read_start((uv_stream_t *)&client_handle, allocate_read_buffer, on_server_message);
}

int main() {
    loop = uv_default_loop();

    uv_signal_t sigint;
    uv_signal_init(loop, &sigint);
    uv_signal_start(&sigint, handle_sigint, SIGINT);

    // Prompt for user's name
    printf(CLEAR_SCREEN);
    print_chat_title();
    printf("\nEnter your name: ");
    fgets(user_name, sizeof(user_name), stdin);
    user_name[strcspn(user_name, "\n")] = 0;

    // Initial chat display
    display_chat_log(NULL);

    // Set up user input pipe
    uv_pipe_init(loop, &stdin_pipe, NO_IPC);
    uv_pipe_open(&stdin_pipe, STDIN_FILENO);
    uv_read_start((uv_stream_t *)&stdin_pipe, allocate_read_buffer, on_user_input);

    // Connect to the server
    uv_connect_t connection_req;
    connect_to_server(&connection_req);
    uv_run(loop, UV_RUN_DEFAULT);

    return EXIT_SUCCESS;
}
