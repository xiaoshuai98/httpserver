add_executable(test_buffer test_buffer.c)
target_link_libraries(test_buffer PRIVATE httpserver)

add_executable(test_event test_event.c)
target_link_libraries(test_event PRIVATE httpserver)
