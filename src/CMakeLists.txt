# Set source files for server and client
set(SERVER_SOURCES
    server.c
    list.c
    serialization.c
)

set(CLIENT_SOURCES
    client.c
    list.c
    serialization.c
)

# Add server executable
add_executable(server ${SERVER_SOURCES})

# Add client executable
add_executable(client ${CLIENT_SOURCES})

# Add the include directory for list.h
include_directories(${CMAKE_SOURCE_DIR}/include)

# Link the libuv library to both server and client
target_include_directories(server PRIVATE ${LIBUV_INCLUDE_DIRS})
target_include_directories(client PRIVATE ${LIBUV_INCLUDE_DIRS})

target_link_libraries(server PRIVATE ${LIBUV_LIBRARIES})
target_link_libraries(client PRIVATE ${LIBUV_LIBRARIES})
