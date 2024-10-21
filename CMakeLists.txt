cmake_minimum_required(VERSION 3.30)
project(WhisperWorld LANGUAGES C)

set(CMAKE_C_STANDARD 23)

# Include the Dependencies (libuv)
include(cmake/Dependencies.cmake)

# Add the src directory, which contains its own CMakeLists.txt
add_subdirectory(src)

# Optional: Clean the build directory
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Clean and remove the build directory"
)
