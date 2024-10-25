cmake_minimum_required(VERSION 3.30)
project(WhisperWorld LANGUAGES C)

set(CMAKE_C_STANDARD 23)

# Enable testing with CTest
enable_testing()

# Include include dir globally for headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Define source files
set(SOURCES
    src/list.c
)

include(cmake/Dependencies.cmake)

add_subdirectory(src)
add_subdirectory(tests)

# Optional: Clean the build directory
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Clean and remove the build directory"
)
