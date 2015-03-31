
#include "Loop.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#include <functional>

using namespace std;
using namespace std::placeholders;

/* int p1[2], p2[2], p3[2]; */
int p[3][2];
Loop loop;
int counter = 0;

void print_events(int i, uint32_t events) {
    cout << "events for " << p[i][0] << ": ";
    if (events & EPOLLIN) cout << "EPOLLIN ";
    if (events & EPOLLOUT) cout << "EPOLLOUT ";
    if (events & EPOLLRDHUP) cout << "EPOLLRDHUP ";
    if (events & EPOLLPRI) cout << "EPOLLPRI ";
    if (events & EPOLLERR) cout << "EPOLLERR ";
    if (events & EPOLLHUP) cout << "EPOLLHUP ";
    cout << endl;
    counter++;
    close(p[i][1]);
    if (counter > 10) exit(0);
}

void del_other_pipes(int index, uint32_t events) {

    for (int i = 0; i <= 2; i++) {
        if (i != index) {
            loop.del_fd(p[i][0]);
            close(p[i][0]);
        }
    }

    int val;
    read(p[index][0], &val, sizeof(val));
    cout << "Pipe: " << index << ", val: " << val << endl;
    if (val == 0) loop.exit_loop();

}

template<typename T>
T f(T t) { return t; }

template<typename T=int>
bool f(int i) { return i > 0; }

template<typename T=bool>
int f(bool i) { return 0; }

template<typename T>
auto f(T t) -> decltype(f<T>(t)) { return f<T>(t); }

int main() {
    //Run a simple test with some pipes

    pipe(p[0]);
    pipe(p[1]);
    pipe(p[2]);

    for (int i = 0; i <= 2; i++) {
        loop.add_fd(p[i][0], EPOLLIN, bind(print_events, i, _1));
    }
    int val = 1;
    for (int i = 0; i <= 2; i++) {
        write(p[i][1], &val, sizeof(val));
        //close(p[i][1]);
    }
    /* val = 0; */
    /* for (int i = 0; i <= 2; i++) { */
    /*     write(p[i][1], &val, sizeof(val)); */
    /* } */
    loop.handle_events();
    return 0;
}
