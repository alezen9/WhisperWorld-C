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

typedef struct {
    int idx;            // Client slot index
    uv_tcp_t handle;    // TCP handle for the client
    struct Message msg; // Message structure for each client
} client_t;

client_t *clients[MAX_CLIENTS];

// Finds a free slot in clients array for a new connection
int find_free_client_slot() {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if (clients[i] == NULL) return i;
    return -1;
}

// Deallocates client and clears slot on disconnect
void free_client_slot(client_t *client) {
    printf("Client at idx %d disconnected\n", client->idx);
    clients[client->idx] = NULL;
    free(client);
}

void on_client_disconnect(uv_handle_t *handle) {
    client_t *client = (client_t *)handle->data;
    free_client_slot(client);
}

// Callback after message writing completes, frees write request memory
void on_write_end(uv_write_t *req, int status) {
    if (status) fprintf(stderr, "Write error: %s\n", uv_strerror(status));
    free(req);
}

// Broadcasts message to all clients except the sender
void broadcast_message(uv_tcp_t *sender, struct Message *msg) {
    char buffer[sizeof(struct Message)];
    if (serialize_message(msg, buffer, sizeof(buffer)) < 0) {
        fprintf(stderr, "Serialization error\n");
        return;
    }
    uv_buf_t buf = uv_buf_init(buffer, sizeof(buffer));

    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] == NULL || (uv_tcp_t *)&clients[i]->handle == sender) continue;

        uv_write_t *req = malloc(sizeof(uv_write_t));
        if (!req) {
            fprintf(stderr, "Memory allocation failed for write request\n");
            continue;
        }

        // Perform the write and free on completion
        if (uv_write(req, (uv_stream_t *)&clients[i]->handle, &buf, 1, on_write_end)) {
            fprintf(stderr, "Error sending message to client %d\n", i);
            free(req);
        }
    }
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    buf->base = malloc(suggested_size);
    if (!buf->base) {
        fprintf(stderr, "Error allocating memory\n");
        exit(EXIT_FAILURE);
    }
    buf->len = suggested_size;
}

// Reads and processes incoming data from a client
void on_read(uv_stream_t *client_stream, ssize_t nread, const uv_buf_t *buf) {
    client_t *client = (client_t *)client_stream->data;
    struct Message *msg = &client->msg;
    memset(msg, 0, sizeof(struct Message)); // Clear any previous data

    if (nread < 0 || nread == UV_EOF) {
        if (nread != UV_EOF) fprintf(stderr, "Read error: %s\n", uv_strerror(nread));
        uv_close((uv_handle_t *)client_stream, on_client_disconnect);
        free(buf->base);
        return;
    }

    if (nread == 0) {
        free(buf->base);
        return;
    }

    if (deserialize_message(buf->base, msg, nread) < 0) {
        fprintf(stderr, "Deserialization error\n");
        free(buf->base);
        return;
    }

    if (strcmp(msg->content, "quit") == 0) {
        printf("Client %d sent 'quit', closing connection.\n", client->idx);
        uv_close((uv_handle_t *)client_stream, on_client_disconnect);
        free(buf->base);
        return;
    }

    printf("[%s] %s\n", msg->user_name, msg->content);
    broadcast_message((uv_tcp_t *)client_stream, msg);

    free(buf->base);
}

void on_new_connection(uv_stream_t *server, int status) {
    if (status < 0) {
        fprintf(stderr, "New connection error: %s\n", uv_strerror(status));
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

    clients[free_slot] = client;
    client->idx = free_slot;
    client->handle.data = client;

    if (uv_tcp_init(uv_default_loop(), &client->handle) < 0) {
        fprintf(stderr, "Failed to initialize TCP handle.\n");
        free_client_slot(client);
        return;
    }

    if (uv_accept(server, (uv_stream_t *)&client->handle) != 0) {
        uv_close((uv_handle_t *)&client->handle, NULL);
        free_client_slot(client);
        return;
    }

    printf("New client connected\n");

    if (uv_read_start((uv_stream_t *)&client->handle, alloc_buffer, on_read) < 0)
        fprintf(stderr, "Error starting to read\n");
}

int main() {
    uv_tcp_t server;
    uv_loop_t *loop = uv_default_loop();

    uv_tcp_init(loop, &server);

    struct sockaddr_in addr;
    uv_ip4_addr("0.0.0.0", PORT, &addr);
    uv_tcp_bind(&server, (const struct sockaddr *)&addr, 0);

    if (uv_listen((uv_stream_t *)&server, CONNECTION_REQUEST_QUEUE_SIZE, on_new_connection)) {
        fprintf(stderr, "Listen error\n");
        return 1;
    }

    printf("Server listening on port %d...\n", PORT);
    return uv_run(loop, UV_RUN_DEFAULT);
}
