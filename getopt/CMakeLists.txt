cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

project("NetBSD implementation of POSIX getopt()")

add_library (getopt STATIC
    "getopt.c"
    "getopt.h"
)
target_include_directories(getopt
    PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}
)