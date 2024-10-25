# test_list
# Add an executable for test_list.c
add_executable(test_list test_list.c ${CMAKE_SOURCE_DIR}/src/list.c)

# Link the test executable with `Check` and the `list.c` source code
target_link_libraries(test_list PRIVATE ${CHECK_LIBRARIES})

# Register the executable as a test with CTest
add_test(NAME test_list COMMAND test_list)

# add lines above for each test file