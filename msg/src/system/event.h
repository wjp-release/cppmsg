#pragma once

#include "def.h"
#include "reactor.h"

namespace msg{ 

class event{
public:
    event(int epollfd, int fd, const event_cb& cb);
    ~event();
    int             fd; 
    bool            submit(int evflag); 
    bool            submit_in();
    bool            submit_out();
    bool            submit_both();
    void            please_destroy_me(); // safe destruction of event object that notifies the eventloop to delete event, which happens after current epoll_wait
    void            set_cb(event_cb);
    void            consume(int evflag); // must be called by eventloop
    bool            submit_without_oneshot(int evflag);  // must be called by eventloop; timerfd, eventfd & signalfd
private:
    event_cb        cb;
    int             evmask  = 0; //what's under watch
    bool            closing = false; 
    std::mutex      mtx;
};


}