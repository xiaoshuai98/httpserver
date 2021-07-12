file (GLOB LIB_SOURCES "*.c")

add_library (httpserver STATIC ${LIB_SOURCES})

if (CMAKE_C_COVERAGE)
  target_link_libraries(httpserver PRIVATE gcov)
endif()
