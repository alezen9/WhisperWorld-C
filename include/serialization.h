#ifndef SERIALIZATION_H
#define SERIALIZATION_H

#include "list.h" // Include your Message structure definition

// Function to serialize a Message struct into a byte buffer
int serialize_message(const struct Message* msg, char* buffer, size_t buffer_size);

// Function to deserialize a byte buffer into a Message struct
int deserialize_message(const char* buffer, struct Message* msg, size_t buffer_size);

#endif // SERIALIZATION_H
