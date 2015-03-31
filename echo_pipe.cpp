
#include "Loop.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#include <future>
#include <functional>

#define CLIENTS 10
#define FAN 50
using namespace std;
using namespace std::placeholders;

int counter = 0;
int done = 0;
Loop loop;
int poll_count = 0;

int pin[CLIENTS][FAN][2];
int pout[CLIENTS][FAN][2];

void count_poll(int a, int b) {
    poll_count++;
}

void echo_int(int id, int j, uint32_t events) {
    int buf;
    int n;
    if (events & EPOLLIN) {
        n = read(pin[id][j][0], &buf, sizeof(buf));
        if (n < sizeof(buf)) {
            cout << "short read" << endl;
            return;
        }
        write(pout[id][j][1], &buf, sizeof(buf));
    } else {
        // If there is nothing to be read, close the pipe
        /* if (events & EPOLLIN) cout << "EPOLLIN "; */
        /* if (events & EPOLLOUT) cout << "EPOLLOUT "; */
        /* if (events & EPOLLRDHUP) cout << "EPOLLRDHUP "; */
        /* if (events & EPOLLPRI) cout << "EPOLLPRI "; */
        /* if (events & EPOLLERR) cout << "EPOLLERR "; */
        /* if (events & EPOLLHUP) cout << "EPOLLHUP "; */
        /* cout << endl; */
        loop.del_fd(pin[id][j][0]);
        loop.del_fd(pout[id][j][1]);
        /* cout << "Closing: " << id << " " << j << endl; */
        if (++done == (CLIENTS * FAN)) {
            loop.exit_loop();
            /* cout << "exiting" << endl; */
        }
    }
}

void echo_client(int id, int count) {
    int buf;
    for (int i = 0; i < count; i++) {
        for (int j = 0; j < FAN; j++) {
            if (write(pin[id][j][1], &i, sizeof(i)) < sizeof(i)) {
                cout << "problem writing" << endl;
            }
        }
        for (int j = 0; j < FAN; j++) {
            if (read(pout[id][j][0], &buf, sizeof(buf)) < sizeof(buf)) {
                cout << "problem reading" << endl;
            }
        }
    }
    for (int i = 0; i < FAN; i++) {
        close(pin[id][i][1]);
        close(pout[id][i][0]);
    }
    cout << "Done" << id << endl;
}

void derp() {
}

int main() {
    loop.set_default(&count_poll);
    for (int i = 0; i < CLIENTS; i++) {
        for (int j = 0; j < FAN; j++) {
            if (pipe(pin[i][j]) == -1) {
                perror("pipe");
                exit(1);
            }
            if (pipe(pout[i][j]) == -1) {
                perror("pipe");
                exit(1);
            }
            if (loop.add_fd(pin[i][j][0], EPOLLIN, bind(echo_int, i, j, _1)) == -1) exit(1);
        }
    }
    loop.queue_event(timeval{10,0}, &derp, "");
    std::future<void> result[CLIENTS];
    std::future<int> server(std::async(std::launch::async, bind(&Loop::handle_events, ref(loop))));

    for (int i = 0; i < CLIENTS; i++) {
       result[i] = std::async(std::launch::async, bind(echo_client, i, 1000));
    }
    for (int i = 0; i < CLIENTS; i++) {
        result[i].get();
    }
    server.get();
    cout << poll_count << endl;
    return 0;
}
