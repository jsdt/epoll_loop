cmake_minimum_required(VERSION 3.1)
project(epoll_loop)

include (CheckSymbolExists)
check_symbol_exists(epoll_create "sys/epoll.h" EPOLL_EXISTS)
if (NOT EPOLL_EXISTS)
    message(FATAL_ERROR "system does not have epoll")
endif(NOT EPOLL_EXISTS)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g -Og")

#set(CMAKE_EXE_LINKER_FLAGS "-static")
add_library(LOOP STATIC Loop.cpp Loop.h)
set_target_properties(LOOP PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -Wall ")

add_executable(pipe_test pipe_test.cpp Loop.h)
set_target_properties(pipe_test PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -w")
target_link_libraries(pipe_test LOOP)

add_executable(pipe_bench echo_pipe.cpp Loop.h)
add_executable(tcp_server tcp_server.cpp net_include.h time_util.h Loop.h)
add_executable(tcp_client tcp_client.c net_include.h)
target_link_libraries(pipe_bench LOOP)
target_link_libraries(tcp_server LOOP)
