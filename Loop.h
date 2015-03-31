//
// Created by jeffrey on 3/17/15.
//

#ifndef _EPOLL_TCP_EPOLLLOOP_H_
#define _EPOLL_TCP_EPOLLLOOP_H_

#include "net_include.h"
#include <sys/time.h>
#include <set>
#include <forward_list>
#include <map>
#include <functional>
#include <memory>
#include "time_util.h"
#include <vector>

using namespace std;

using fd_callback = function<void (uint32_t)>;

namespace loop_help
{
    template<typename T>
    class xref_wrap : public std::reference_wrapper<T>
    {
    public:
        xref_wrap(T& x) : std::reference_wrapper<T>(x) {}
        xref_wrap(const reference_wrapper<T>& x) : std::reference_wrapper<T>(x) {}
        xref_wrap(const xref_wrap<T>& other) : std::reference_wrapper<T>(static_cast<const std::reference_wrapper<T>&>(other)) {}
        bool operator<(xref_wrap const rhs) const {
            return this->get() < rhs.get();
        }   
    };
}

class Loop {
public:
    Loop();
    ~Loop();
    //Will be called with the number of events returned by epoll_wait and the number of timers that were called
    void set_default(function<void (int events, int timers_called)> cb);
    void set_poll_timeout(int millis);
    int add_fd(int fd, uint32_t events, fd_callback cb);
    void change_cb(int fd, fd_callback cb);
    int mod_fd(int fd, uint32_t events, fd_callback cb);
    int del_fd(int fd);
    void queue_event(timeval delta, function<void ()>, const string& id);
    void remove_event(const string& id);
    int handle_events();
    void exit_loop();

private:
    int efd;
    const static int MAX_EVENTS = 250;
    map<int, shared_ptr<fd_callback>> fd_to_cb;
    function<void (int, int)> default_cb;
    map<pair<timeval, loop_help::xref_wrap<const string>>, tuple<function<void ()>, map<loop_help::xref_wrap<const string>, timeval>::iterator, shared_ptr<string>>> timers;
    map<loop_help::xref_wrap<const string>, timeval> id_to_timer;
    forward_list<shared_ptr<fd_callback>> deleted_callbacks;
    set<int> deleted_fds;
    bool done;
    bool handling_events;
    int poll_timeout;

};


#endif //_EPOLL_TCP_EPOLLLOOP_H_
