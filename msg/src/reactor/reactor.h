#pragma once

#include "def.h"

#include <mutex>
#include <vector>
#include <thread>
#include "timer.h"

struct epoll_event;
namespace msg{ namespace reactor{
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
    void            eventloop(); // the eventloop
    // Submit a task and run in the eventloop
    // Do not abuse this feature. We do not intend to design the eventloop as a task pool.
    // The cbq is designed for destroying events and handling timer updates, as these two operations deserve careful synchronization with epoll_waits. 
    void            submit_and_wake(const please_cb& cb);
    void            submit(const please_cb& cb);
    void            wake(); // via sending eventfd
    timer&          get_timer()noexcept{
        return timerfd_timer;
    }
protected:
    reactor();
    void            bad_epollwait();
    void            consume(const struct epoll_event* ev);
    bool            in_eventloop();
private:
    int             eventfd; // used to wake up eventloop
    event*          eventfd_event; // wake up event
    std::thread     background_thread;   
    cq<please_cb>   cbq; // callbacks to run in next loop 
    timer           timerfd_timer; // timerfd-based timer
    event*          timerfd_event; 
    // event*          signalfd_event;  
    bool            closing = false; 
};


}}