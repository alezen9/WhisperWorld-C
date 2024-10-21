#include "serialization.h"
#include <string.h> // For memcpy

#define USERNAME_SIZE 32 // Assume user_name max length
#define CONTENT_SIZE 256 // Assume content max length

// Serialize a Message into a byte buffer
int serialize_message(const struct Message *msg, char *buffer, size_t buffer_size) {
    if (buffer_size < sizeof(struct Message)) return -1; // Buffer too small

    // Serialize: Copy fields one by one to ensure structure order
    size_t offset = 0;

    memcpy(buffer + offset, msg->user_name, USERNAME_SIZE);
    offset += USERNAME_SIZE;

    memcpy(buffer + offset, msg->content, CONTENT_SIZE);
    offset += CONTENT_SIZE;

    memcpy(buffer + offset, &msg->timestamp, sizeof(msg->timestamp));

    return 0; // Success
}

// Deserialize a byte buffer into a Message struct
int deserialize_message(const char *buffer, struct Message *msg, size_t buffer_size) {
    if (buffer_size < sizeof(struct Message)) return -1; // Buffer too small

    // Deserialize: Read fields in the correct order
    size_t offset = 0;

    memcpy(msg->user_name, buffer + offset, USERNAME_SIZE);
    offset += USERNAME_SIZE;

    memcpy(msg->content, buffer + offset, CONTENT_SIZE);
    offset += CONTENT_SIZE;

    memcpy(&msg->timestamp, buffer + offset, sizeof(msg->timestamp));

    return 0; // Success
}
