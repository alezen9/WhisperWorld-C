# Use FetchContent to download and build external dependencies
include(FetchContent)

# Fetch libuv if not already installed
FetchContent_Declare(
  libuv
  GIT_REPOSITORY https://github.com/libuv/libuv.git
  GIT_TAG v1.49.2  # Use a specific stable version
)
FetchContent_MakeAvailable(libuv)

# Make libuv available globally
set(LIBUV_INCLUDE_DIRS ${libuv_SOURCE_DIR}/include)
set(LIBUV_LIBRARIES uv_a)

FetchContent_Declare(
  check
  GIT_REPOSITORY https://github.com/libcheck/check.git
  GIT_TAG 0.15.2  # Use a specific version if preferred
)
FetchContent_MakeAvailable(check)

# Make check available globally
set(CHECK_INCLUDE_DIRS ${check_SOURCE_DIR}/include)
set(CHECK_LIBRARIES check)