cmake_minimum_required(VERSION 3.7)
project(surveillance)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(surveillance ${SOURCE_FILES})
add_executable(helloworld proto.c)
