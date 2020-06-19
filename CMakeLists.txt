cmake_minimum_required(VERSION 3.15)

# library
project(arg_parse_convert)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++17")

add_library(arg_parse_convert
        "${CMAKE_CURRENT_SOURCE_DIR}/src/parameter.cc"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/parameter_map.cc")
target_include_directories(arg_parse_convert PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include")

# unit tests
project(arg_parse_convert_test)
include(CTest)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")