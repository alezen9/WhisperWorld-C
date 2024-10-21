#include "list.h"
#include "serialization.h"
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uv.h> // Include libuv

#define CONNECTION_REQUEST_QUEUE_SIZE 3
#define PORT 8080
#define MAX_CLIENTS 50

// Structure to hold client data
typedef struct {
    int idx;            // index of occupied slot within clients array
    uv_tcp_t handle;    // The TCP handle for the client
    struct Message msg; // The message structure for this client
} client_t;

// Global array to store client connections
client_t *clients[MAX_CLIENTS];

// Function to find a free slot in the clients array
int find_free_client_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i] == NULL) return i; // Return the first free slot index
    return -1;                            // No free slots available
}

void on_client_disconnect(uv_handle_t *handle) {
    client_t *client = (client_t *)handle->data;

    printf("Client (idx = %d) disconnected\n", client->idx);
    clients[client->idx] = NULL; // Free the slot in the array

    free(client); // Free the allocated memory for the client
}

void on_write_end(uv_write_t *req, int status) {
    if (status) fprintf(stderr, "Write error: %s\n", uv_strerror(status));
    free(req); // Free memory after the write is done
}

// Function to broadcast messages to other clients
void broadcast_message(uv_tcp_t *sender, struct Message *msg) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        // Check if the client is active and not the sender
        if (clients[i] == NULL || (uv_tcp_t *)&clients[i]->handle == sender) continue;

        uv_write_t *req = (uv_write_t *)malloc(sizeof(uv_write_t)); // Allocate memory for the write request
        if (req == NULL) {
            fprintf(stderr, "Memory allocation failed for write request.\n");
            continue;
        }

        char buffer[sizeof(struct Message)];

        // Serialize the message before sending
        if (serialize_message(msg, buffer, sizeof(struct Message)) < 0) {
            printf("Serialization error\n");
            free(req);
            continue;
        }

        uv_buf_t buf = uv_buf_init(buffer, sizeof(struct Message)); // Create a buffer from the message

        // Send the message to the client, and pass a callback to free the write request after writing
        if (uv_write(req, (uv_stream_t *)&clients[i]->handle, &buf, 1, on_write_end)) {
            fprintf(stderr, "Error sending message to client %d\n", i);
            free(req); // Ensure to free the write request in case of failure
        }
    }
}

// Allocate memory for the read buffer
void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = malloc(suggested_size);
    if (!buf->base) {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
    buf->len = suggested_size;
}

// Handle incoming data from a client
void on_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf) {
    if (nread < 0 || nread == UV_EOF) {
        if (nread != UV_EOF) fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
        uv_close((uv_handle_t *)client_stream, on_client_disconnect); // Close the connection if there's an error or EOF
    }
    if (nread == 0) {
        free(buf->base); // Free the buffer memory
        return;
    }

    // Access the client data structure
    client_t *client = (client_t *)client_stream->data;
    struct Message *msg = &client->msg;

    // Deserialize the message from the buffer
    if (deserialize_message(buf->base, msg, nread) < 0) {
        printf("Deserialization error\n");
        return;
    }

    // Print the message and broadcast it to other clients
    printf("[%s] %s\n", msg->user_name, msg->content);
    broadcast_message((uv_tcp_t *)client_stream, msg); // Send the message to all other clients

    free(buf->base); // Free the buffer memory
}

// Function to handle new connections
void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error: %s\n", uv_strerror(status)); // Handle connection errors
        return;
    }

    int free_slot = find_free_client_slot();
    if (free_slot == -1) {
        fprintf(stderr, "Client limit reached, rejecting connection.\n");
        return;
    }

    client_t *client = malloc(sizeof(client_t));
    if (!client) {
        fprintf(stderr, "Memory allocation error\n");
        return;
    }

    clients[free_slot] = client;  // Add the client to the array
    client->idx = free_slot;      // Set the client_id to the free slot
    client->handle.data = client; // Link the handle to the client struct

    int tcp_init_result = uv_tcp_init(uv_default_loop(), &client->handle);
    if (tcp_init_result < 0) {
        fprintf(stderr, "Failed to initialize TCP handle: %s\n", uv_strerror(tcp_init_result));
        free(client); // Free the allocated memory
        clients[free_slot] = NULL;
        return;
    }

    if (uv_accept(server, (uv_stream_t *)&client->handle) != 0) {
        uv_close((uv_handle_t *)&client->handle, NULL); // Close the connection if not accepted
        free(client);                                   // Free the allocated memory
        clients[free_slot] = NULL;
    }

    printf("New client connected\n");

    // Start reading data from the client
    int result = uv_read_start((uv_stream_t *)&client->handle, alloc_buffer, on_read);
    if (result < 0) fprintf(stderr, "Error starting to read: %s\n", uv_strerror(result));
}

int main() {
    uv_tcp_t server;
    uv_loop_t *loop = uv_default_loop(); // Get the default libuv loop

    // Initialize the TCP server
    uv_tcp_init(loop, &server);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", PORT, &addr); // Create an IPv4 address for binding

    // Bind the server to the address and port
    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);

    // Start listening for connections
    int listen_status = uv_listen((uv_stream_t *)&server, CONNECTION_REQUEST_QUEUE_SIZE, on_new_connection);
    if (listen_status) {
        fprintf(stderr, "Listen error: %s\n", uv_strerror(listen_status)); // Handle listen errors
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);

    // Start the event loop
    return uv_run(loop, UV_RUN_DEFAULT);
}
