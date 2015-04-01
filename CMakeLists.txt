cmake_minimum_required(VERSION 3.1)
project(epoll_loop)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -pthread -g -Og")

add_library(LOOP STATIC Loop.cpp Loop.h)
add_executable(tcp_server tcp_server.cpp net_include.h time_util.h Loop.h)
add_executable(tcp_client tcp_client.c net_include.h)
add_executable(pipe_test pipe_test.cpp Loop.h)
add_executable(pipe_bench echo_pipe.cpp Loop.h )
target_link_libraries(pipe_bench LOOP)
target_link_libraries(pipe_test LOOP)
target_link_libraries(tcp_server LOOP)
