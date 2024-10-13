#include "list.h"
#include <arpa/inet.h> // Required for network functions and structures
#include <errno.h>     // For handling error codes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> // Provides access to POSIX operating system API (e.g., close())

#define PORT 8080      // Server port to listen on
#define MAX_CLIENTS 10 // Maximum number of client connections allowed
#define CONNECTION_REQUEST_QUEUE_SIZE 3

void broadcast_message(int sender_fd, struct Message *msg, int *clients, int num_clients) {
    for (int i = 0; i < num_clients; i++) {
        // Send the message to all clients except the sender
        if (clients[i] != 0 && clients[i] != sender_fd) {
            if (send(clients[i], msg, sizeof(struct Message), 0) == -1) perror("Error broadcasting message to client");
        }
    }
}

void handle_incoming_connection_request(const int *server_fd, fd_set *readfds, int *clients) {
    if (FD_ISSET(*server_fd, readfds)) {
        int new_socket;
        struct sockaddr_in address;
        socklen_t addrlen = sizeof(address);

        new_socket = accept(*server_fd, (struct sockaddr *)&address, &addrlen);

        if (new_socket < 0) {
            perror("Accept error");
            exit(EXIT_FAILURE); // Fail fast on accept error
        }

        // Add the new client to the clients array
        int added = 0;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == 0) {
                clients[i] = new_socket;
                printf("New connection, socket fd is %d, ip is: %s, port: %d \n", new_socket,
                       inet_ntoa(address.sin_addr), ntohs(address.sin_port));
                added = 1;
                break;
            }
        }

        if (!added) {
            printf("Client limit reached. Rejecting connection.\n");
            close(new_socket); // Close connection if the client limit is reached
        }
    }
}

void handle_incoming_message_request(fd_set *readfds, int *clients) {
    int client_fd, valread;
    struct Message msg; // Buffer to hold incoming messages
    // Check each client socket for incoming data
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fd = clients[i];

        // If the client socket has activity, process the data
        if (FD_ISSET(client_fd, readfds)) {
            // Read the incoming message
            valread = recv(client_fd, &msg, sizeof(struct Message), 0);

            // If no data was read, the client has disconnected
            if (valread == 0) {
                close(client_fd); // Close the client socket
                clients[i] = 0;   // Mark this slot as empty in the clients array
                printf("Client disconnected\n");
            } else if (valread < 0) {
                perror("recv failed");
            } else {
                // Print and broadcast the received message to other clients
                printf("[%s] %s\n", msg.user_name, msg.content);
                broadcast_message(client_fd, &msg, clients, MAX_CLIENTS);
            }
        }
    }
}

void reset_readfds(int *server_fd, fd_set *readfds) {
    // Clear the socket set
    FD_ZERO(readfds);
    // Add the server socket to the set
    FD_SET(*server_fd, readfds);
}

int wait_for_activity(int *server_fd, fd_set *readfds, int *clients) {
    int max_socket_fd = *server_fd;
    int socket_fd;
    struct timeval timeout;
    timeout.tv_sec = 30; // Timeout of 30 seconds
    timeout.tv_usec = 0;

    // Add all active client sockets to the set
    for (int i = 0; i < MAX_CLIENTS; i++) {
        socket_fd = clients[i];

        // Only add valid socket descriptors to the set
        if (clients[i] > 0) FD_SET(socket_fd, readfds);

        // Track the highest socket descriptor number
        if (socket_fd > max_socket_fd) max_socket_fd = socket_fd;
    }

    // Wait for activity on any socket using select()
    return select(max_socket_fd + 1, readfds, NULL, NULL, &timeout);
}

void process_requests(int *server_fd) {
    int clients[MAX_CLIENTS] = {0}; // Array to keep track of client sockets (0 if empty)
    fd_set readfds;                 // Set of file descriptors to monitor (using select)
    int active_fds_count;           // Number of ready file descriptors and bytes read

    while (1) {
        reset_readfds(server_fd, &readfds);

        active_fds_count = wait_for_activity(server_fd, &readfds, clients);

        // Fail fast on select error
        if ((active_fds_count < 0) && (errno != EINTR)) {
            perror("Select error");
            close(*server_fd);  // Clean up before exiting
            exit(EXIT_FAILURE); // Fail fast if select fails
        }

        handle_incoming_connection_request(server_fd, &readfds, clients);
        handle_incoming_message_request(&readfds, clients);
    }
}

int create_server() {
    int server_fd;
    struct sockaddr_in server_address;
    int enable_reuse_address = 1;

    // Create the server socket (TCP)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE); // Fail fast if socket creation fails
    }

    // Allow reuse of the address to avoid "Address already in use" error
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &enable_reuse_address, sizeof(enable_reuse_address)) < 0) {
        perror("setsockopt failed");
        close(server_fd);   // Ensure resources are cleaned up
        exit(EXIT_FAILURE); // Fail fast if setting socket options fails
    }

    // Configure the server address
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port = htons(PORT);

    // Bind the socket to the specified address and port
    if (bind(server_fd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
        perror("Bind failed");
        close(server_fd);   // Cleanup in case of failure
        exit(EXIT_FAILURE); // Fail fast if binding fails
    }

    return server_fd;
}

int main() {
    int server_fd = create_server(); // If this fails, the program already exits

    // Start listening for incoming connections
    if (listen(server_fd, CONNECTION_REQUEST_QUEUE_SIZE) < 0) {
        perror("Listen failed");
        close(server_fd);   // Cleanup before exit
        exit(EXIT_FAILURE); // Fail fast if listening fails
    }

    printf("Server listening on port %d...\n", PORT);

    process_requests(&server_fd); // Start handling requests

    return EXIT_SUCCESS;
}
