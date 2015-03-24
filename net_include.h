//
// Created by jeffrey on 3/16/15.
//

#ifndef _EPOLL_TCP_NET_INCLUDE_H_
#define _EPOLL_TCP_NET_INCLUDE_H_
#include <stdio.h>

#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netdb.h>

#include <errno.h>

#define PORT         11011

#define MAX_MESS_LEN 8192

#endif //_EPOLL_TCP_NET_INCLUDE_H_
