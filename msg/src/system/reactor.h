#pragma once

#include "def.h"

#include <mutex>
#include <vector>
#include <thread>
#include "timer.h"

/*
The class reactor implements a level-triggered epoll reactor, that could safely distribute event handling tasks to worker threads. 

It has following features:
1. It uses one-shot mode, which ensures that each event can only be processed by one worker thread. 
2. Destructors of event object can only be run by eventloop. The eventloop offers an interface to help release event objects for user threads. Therefore epoll_wait will never return a event of a fd that has just been closed by some random threads.
3. The eventloop is integrated with a timerfd-based cancellable timer, and a singalfd-based singal handling mechanism. 
*/

struct epoll_event;
namespace msg{ 
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
    void            stop();
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

static inline void defer(timer_cb cb, uint64_t delay){
    reactor::instance().get_timer().please_push(cb, future(delay));
}

static inline void repeat(timer_cb cb, uint64_t delay, uint32_t period){
    reactor::instance().get_timer().please_push(cb, future(delay), period);
}


}