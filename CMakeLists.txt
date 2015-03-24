cmake_minimum_required(VERSION 3.1)
project(epoll_loop)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

add_executable(tcp_server tcp_server.cpp net_include.h time_util.h Loop.cpp Loop.h)
add_executable(tcp_client tcp_client.c net_include.h)