file (GLOB LIB_SOURCES "*.c" "lex.yy.c" "parser.tab.c")

add_library (httpserver STATIC ${LIB_SOURCES})

if (CMAKE_C_COVERAGE)
  target_link_libraries(httpserver PUBLIC gcov)
endif()

if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  target_compile_definitions(httpserver PUBLIC YACCDEBUG)
  target_compile_definitions(httpserver PUBLIC LEXDEBUG)
endif()
