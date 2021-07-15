add_executable(test_buffer test_buffer.c)
target_link_libraries(test_buffer PUBLIC httpserver)

add_executable(test_event test_event.c)
target_link_libraries(test_event PUBLIC httpserver)

add_executable(test_parse test_parse.c)
target_link_libraries(test_parse PUBLIC httpserver)
