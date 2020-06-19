cmake_minimum_required(VERSION 3.15)

# unit tests
project(arg_parse_convert_test)
include(CTest)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -std=c++11")

add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/test")