//
// Created by jeffrey on 3/16/15.
//

#ifndef _EPOLL_TCP_TIME_UTIL_H_
#define _EPOLL_TCP_TIME_UTIL_H_

#include <sys/time.h>

int inline compare_time(timeval t1, timeval t2);
bool inline operator<(timeval t1, timeval t2);
bool inline operator<=(timeval t1, timeval t2);
bool inline operator>(timeval t1, timeval t2);
bool inline operator>=(timeval t1, timeval t2);
bool inline operator==(timeval t1, timeval t2);
timeval inline operator+(timeval t1, timeval t2);
timeval inline operator-(timeval t1, timeval t2);

int inline compare_time(timeval t1, timeval t2) {
    if (t1.tv_sec > t2.tv_sec) return 1;
    else if (t1.tv_sec < t2.tv_sec) return -1;
    else if (t1.tv_usec > t2.tv_usec) return 1;
    else if (t1.tv_usec < t2.tv_usec) return -1;
    else return 0;
}

bool inline operator<(timeval t1, timeval t2) {
    return -1 == compare_time(t1, t2);
}

inline bool operator>(timeval t1, timeval t2) {
    return 1 == compare_time(t1, t2);
}

inline bool operator>=(timeval t1, timeval t2) {
    return -1 != compare_time(t1, t2);
}

inline bool operator<=(timeval t1, timeval t2) {
    return 1 != compare_time(t1, t2);
}

inline bool operator==(timeval t1, timeval t2) {
    return (t1.tv_usec == t2.tv_usec) && (t1.tv_sec == t2.tv_sec);
}

timeval inline operator+(timeval t1, timeval t2) {
    if (t1.tv_usec + t2.tv_usec >= 1000000) {
        return timeval{t1.tv_sec + t2.tv_sec + 1, t1.tv_usec + t2.tv_usec - 1000000};
    }
    else {
        return timeval{t1.tv_sec + t2.tv_sec, t1.tv_usec + t1.tv_usec};
    }
}

// This is undefined if t1 < t2
timeval inline operator-(timeval t1, timeval t2) {
    if (t1.tv_usec < t2.tv_usec) {
        return timeval{t1.tv_sec - t2.tv_sec - 1, t1.tv_usec - t2.tv_usec + 1000000};
    }
    else {
        return timeval{t1.tv_sec - t2.tv_sec, t1.tv_usec - t1.tv_usec};
    }
}
#endif //_EPOLL_TCP_TIME_UTIL_H_
