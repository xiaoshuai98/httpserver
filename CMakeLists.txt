cmake_minimum_required(VERSION 3.8)

project(httpserver)

# Modify include paths
include_directories(${PROJECT_SOURCE_DIR}/include)

# Set the Compilation options
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror -Wextra")

# Set the compilation options of the test program
set(CMAKE_DEBUG_C_FLAGS "-fsanitize=address -O1 -fno-omit-frame-pointer")
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${CMAKE_DEBUG_C_FLAGS}")
  set(CMAKE_VERBOSE_MAKEFILE ON)
endif()

# Set the compilation options for code coverage
option(CMAKE_C_COVERAGE "Enable Code Coverage" ON)
if (CMAKE_C_COVERAGE AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -coverage")
endif()

add_subdirectory(src/)

# Compile the test program
if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  add_subdirectory(tests/)
  include(cmake/ctest.cmake)
endif()

# Generate targets for code coverage
if (CMAKE_C_COVERAGE AND ${CMAKE_BUILD_TYPE} STREQUAL "Debug")
  include(cmake/coverage.cmake)
endif()

add_executable(server server.c)

target_link_libraries(server PUBLIC httpserver)
