include(CTest)

# For verbose output, try this command: CTEST_OUTPUT_ON_FAILURE=1 ctest

add_test(NAME "test_buffer" COMMAND ${PROJECT_BINARY_DIR}/tests/test_buffer)
add_test(NAME "test_event" COMMAND ${PROJECT_BINARY_DIR}/tests/test_event)
add_test(NAME "test_parse" COMMAND ${PROJECT_BINARY_DIR}/tests/test_parse)
