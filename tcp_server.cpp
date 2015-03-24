//
// Created by jeffrey on 3/20/15.
//

#include "net_include.h"
#include "Loop.h"
#include <iostream>
#include <functional>

int counter = 0;
using namespace std;
using namespace std::placeholders;

void acceptor(Loop &loop, int s, uint32_t events);
void handle_message(Loop &loop, int s, uint32_t events);
void repeated_timeout(Loop &loop, timeval timeout, const string& id);
void handle_timeout(int, int);

int main(void) {
    struct sockaddr_in name;
    int                s;
    char               mess_buf[MAX_MESS_LEN];
    long               on=1;

    int efd;
    s = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (s<0) {
        perror("Net_server: socket");
        exit(1);
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(on)) < 0)
    {
        perror("Net_server: setsockopt error \n");
        exit(1);
    }

    name.sin_family = AF_INET;
    name.sin_addr.s_addr = INADDR_ANY;
    name.sin_port = htons(PORT);

    if ( bind( s, (struct sockaddr *)&name, sizeof(name) ) < 0 ) {
        perror("Net_server: bind");
        exit(1);
    }

    if (listen(s, 4) < 0) {
        perror("Net_server: listen");
        exit(1);
    }
    Loop loop;
    auto cb = bind(acceptor, std::ref(loop), s, _1);
    loop.add_fd(s, EPOLLIN | EPOLLET, cb);
    loop.set_default(&handle_timeout);
    loop.set_poll_timeout(5000);
    loop.queue_event(timeval{10,0}, bind(repeated_timeout, ref(loop), timeval{15,0}, "1"), "1");
    loop.handle_events();
    cout << "got out" << endl;
    exit(0);
}

void acceptor(Loop &loop, int s, uint32_t events) {
    cout << "got called!" << endl;
    struct sockaddr in_addr;
    socklen_t in_len;
    int infd;

    in_len = sizeof(in_addr);
    for (;;) {
        /* infd = accept4(s, &in_addr, &in_len, SOCK_NONBLOCK); */
        infd = accept(s, 0, 0);
        if (infd == -1) {
            if ((errno == EAGAIN) || (errno == EWOULDBLOCK)) {
                return;
            } else {
                perror("accept");
                loop.del_fd(s);
            }
        }
        cout << "Accepted new connection" << endl;
        loop.add_fd(infd, EPOLLIN, bind(handle_message, ref(loop), infd, _1));
    }
}

void handle_message(Loop &loop, int s, uint32_t events) {
    char buf[1024];
    int n;
    uint32_t mess_len;
    if ((events & EPOLLERR) || (events & EPOLLHUP) || (!(events & EPOLLIN))) {
        perror("epoll");
        close(s);
        loop.del_fd(s);
    }
    n = recv(s, &mess_len, sizeof(mess_len), 0);
    if (n <= 0) {
        cout << "Closing socket: " << s << endl;
        close(s);
        loop.del_fd(s);
        return;
    }
    n = recv(s, buf, mess_len, 0);
    if ( n < mess_len ) {
        close(s);
        loop.del_fd(s);
    }
    buf[mess_len] = '\0';
    printf("socket is %d, message is %s\n", mess_len, buf);
}

void handle_timeout(int n, int t) {
    if (n == 0 && t == 0) {
        cout << "..." << endl;
    }
}

void repeated_timeout(Loop &loop, timeval timeout, const string &id) {
    cout << "Hit repeated timeout: " << id << endl;
    loop.queue_event(timeout, bind(repeated_timeout, ref(loop), timeout, id), id);
}

