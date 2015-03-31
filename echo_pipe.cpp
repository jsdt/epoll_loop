
#include "Loop.h"
#include <unistd.h>
#include <stdio.h>
#include <iostream>

#include <future>
#include <functional>

#define CLIENTS 4
#define FAN 20
using namespace std;
using namespace std::placeholders;

int counter = 0;
int done = 0;
Loop loop;

int pin[CLIENTS][FAN][2];
int pout[CLIENTS][FAN][2];

void bad_default(shared_ptr<Loop>, int a, int b) {
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
        if (j == 2) {
            // Do this just to test deleting fds while there are pending events for that fd
            loop.del_fd(pin[id][j+1][0]);
            loop.add_fd(pin[id][j+1][0], EPOLLIN, bind(echo_int, id, j+1, _1));
            loop.del_fd(pin[id][j+1][0]);
            loop.add_fd(pin[id][j+1][0], EPOLLIN, bind(echo_int, id, j+1, _1));
        }
        write(pout[id][j][1], &buf, sizeof(buf));
    } else {
        loop.del_fd(pin[id][j][0]);
        loop.del_fd(pout[id][j][1]);
        if (++done == (CLIENTS * FAN)) {
            loop.exit_loop();
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
    int poll_counter = 0;
    auto inc_counter = [&poll_counter](int a, int b) { poll_counter++; };
    //fd_callback *fdcp = new function<void (uint32_t)>(bind(echo_int, 1, 1, _1));
    loop.set_default(inc_counter);
    auto fp = make_shared<function<void (int,int)>>([&poll_counter](int a, int b) { poll_counter++; });
    loop.set_default([&poll_counter](int a, int b) { poll_counter++; });
    // The next two lines create a memory leak. Could get rid of shared_ptrs inside of Loop, or could just require that people only make unique ptrs to Loops?
    /* shared_ptr<Loop> lp = make_shared<Loop>(); */
    /* lp->set_default(bind(bad_default, lp, _1, _2)); */
    //loop.set_default(&count_poll);
    int pipes_opened = 0;
    for (int i = 0; i < CLIENTS; i++) {
        for (int j = 0; j < FAN; j++) {
            if (pipe(pin[i][j]) == -1) {
                cout << pipes_opened << endl;
                perror("pipe");
                exit(1);
            }
            pipes_opened++;
            if (pipe(pout[i][j]) == -1) {
                cout << pipes_opened << endl;
                perror("pipe");
                exit(1);
            }
            pipes_opened++;
            if (loop.add_fd(pin[i][j][0], EPOLLIN, bind(echo_int, i, j, _1)) == -1) exit(1);
        }
    }

    int capture;
    std::function<void (int, int)> derp2 = [&capture](int a, int b) { cout << a << endl; };
    loop.queue_event(timeval{10,0}, &derp, "");
    std::future<void> result[CLIENTS];
    std::future<int> server(std::async(std::launch::async, bind(&Loop::handle_events, ref(loop))));

    int iters = 1000;
    for (int i = 0; i < CLIENTS; i++) {
       result[i] = std::async(std::launch::async, bind(echo_client, i, iters));
    }
    for (int i = 0; i < CLIENTS; i++) {
        result[i].get();
    }
    server.get();
    cout << "poll count: " << poll_counter << endl;
    cout << "total ints echo'd: " << (CLIENTS * FAN * iters) << endl;

    return 0;
}
