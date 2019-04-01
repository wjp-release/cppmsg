#pragma once

#include "def.h"

#include <mutex>
#include <vector>
#include <thread>
#include "timer.h"

struct epoll_event;
namespace msg{ namespace posix{
class event;
class reactor{
public:
    reactor(const reactor&) = delete;
    void operator=(const reactor&) = delete;
    ~reactor();
    int epollfd;
    static reactor& instance(){
        static reactor r;
        return r;
    }
    void            start_eventloop(); 
    // queue it, wake event loop immediately
    void            run(const func& cb);
    // queue it, run in next loop
    void            run_later(const func& cb);
protected:
    reactor();
    void            eventloop(); // the eventloop
    void            wake(); // via sending eventfd
    void            consume(const struct epoll_event* ev);
    bool            in_eventloop();
    void            enqueue(const func& cb);
private:
    int             eventfd; // used to wake up eventloop
    event*          eventfd_event; // wake up event
    std::thread     background_thread;   
    std::vector<func> cbq; // callbacks to run in next loop
    std::mutex      mtx; // protect cbq
    timer           timerfd_timer; // timerfd-based timer
    event*          timerfd_event; 
    event*          signalfd_event; 
    bool            closing = false; 
};


}}