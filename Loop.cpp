//
// Created by jeffrey on 3/17/15.
//

#include "Loop.h"

#include <iostream>
#include <memory>

Loop::Loop()
    : poll_timeout(-1),
        handling_events(false) {
    efd = epoll_create1(0);
    if (efd == -1) {
        //Error
    }
}

Loop::~Loop() {
}

int Loop::add_fd(int fd, uint32_t events, fd_callback cb) {
    shared_ptr<fd_callback> ptr = make_shared<fd_callback>(cb);

    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    event.data.ptr = (void *) ptr.get();
    if (-1 == epoll_ctl(efd, EPOLL_CTL_ADD, fd, &event)) {
        return -1;
    }
    fd_to_cb.insert(std::make_pair(fd, ptr));
    return 0;
}

int Loop::mod_fd(int fd, uint32_t events, fd_callback cb) {
    auto old = fd_to_cb[fd];
    *old = cb;
    epoll_event event;
    event.events = events;
    event.data.fd = fd;
    event.data.ptr = (void *) old.get();
    if (-1 == epoll_ctl(efd, EPOLL_CTL_MOD, fd, &event)) {
        fd_to_cb.erase(fd);
        return -1;
    }
    return 0;
}

void Loop::set_poll_timeout(int millis) {
    poll_timeout = millis;
}

void Loop::change_cb(int fd, fd_callback cb) {
    *(fd_to_cb[fd]) = cb;
}

int Loop::del_fd(int fd) {
    auto old = fd_to_cb.find(fd);
    if (old == fd_to_cb.end()) return 0;
    fd_to_cb.erase(old);
    if (!handling_events) deleted_fds.insert(fd);
    return epoll_ctl(efd, EPOLL_CTL_DEL, fd, NULL);
}

void Loop::remove_event(const string &id) {
    auto it = id_to_timer.find(id);
    if (it == id_to_timer.end()) return;
    auto key = timers.find(make_pair(it->second, it->first));
    timers.erase(key);
}

int Loop::handle_events() {
    int n, timeout, timers_called;
    timeval now, next_timer;
    epoll_event events[Loop::MAX_EVENTS] = {0};
    done = false;
    for (;;) {
        timers_called = 0;
        if (timers.size() > 0) {
            gettimeofday(&now, NULL);
            for (auto it = timers.begin(); it != timers.end() && it->first.first < now; it = timers.begin()) {
                auto cb = it->second.first;
                id_to_timer.erase(it->second.second);
                timers.erase(it);
                cb();
                if (done) return 0;
                timers_called++;
            }
        }
        if (timers.size() > 0) {
            next_timer = timers.begin()->first.first - now;
            timeout = (1000 * next_timer.tv_sec) + (next_timer.tv_usec / 1000);
            if (poll_timeout > -1 && poll_timeout < timeout) {
                timeout = poll_timeout;
            }
        } else {
            timeout = poll_timeout;
        }
        if (fd_to_cb.size() > 0) {
            n = epoll_wait(efd, events, Loop::MAX_EVENTS - 1, timeout);
            handling_events = true;
            deleted_fds.clear();
            if (n == -1) {
                //Oh No!
                return -1;
            }
            for (int i = 0; i < n; i++) {
                if (deleted_fds.find(events[i].data.fd) != deleted_fds.end()) continue;
                auto cb = (fd_callback *) events[i].data.ptr;
                (*cb)(events[i].events);
                if (done) {
                    handling_events = false;
                    return 0;
                }
            }
            handling_events = false;
            if (default_cb) default_cb(n, timers_called);
            if (done) return 0;
        } else if (default_cb) {
            default_cb(0, timers_called);
            if (done) return 0;
        }
    }
}

void Loop::exit_loop() {
    done = true;
}

void Loop::set_default(std::function<void(int, int)> cb) {
    default_cb = cb;
}

void Loop::queue_event(timeval delta, std::function<void ()> cb, const string &id) {
    timeval now;
    gettimeofday(&now, NULL);
    delta = delta + now;
    auto ins = id_to_timer.insert(make_pair(id, delta));
    if (!ins.second) return;
    timers.insert(make_pair(make_pair(delta, id), make_pair(cb, ins.first)));
}

