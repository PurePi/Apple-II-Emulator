cmake_minimum_required(VERSION 3.12)
project(testCard C)

set(CMAKE_C_STANDARD 11)

add_library(testCard SHARED library.c library.h)

set_target_properties(testCard PROPERTIES PREFIX "")