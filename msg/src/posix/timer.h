#pragma once

#include "def.h"
#include <queue>
#include <stdint.h>
#include <functional>
#include <vector>
#include <unistd.h> // close

#include "clock.h"
#include "fd.h"

// timeout: encapsulation of when, and what to do 
// timer: a compact and fast timerfd-based timer

namespace msg{ namespace posix{

struct timeout{
    timeout(const timer_cb& cb, uint64_t expire, uint32_t interval):cb(cb), expire(expire), interval(interval){}
    void        refresh(uint64_t baseline){
        expire=baseline+interval;
    }
    void        refresh_now(){
        expire=now()+interval;
    }
    bool        is_periodic(){
        return interval>0;
    }
    timer_cb        cb; // cb must finish very fast or run in a background thread
    uint64_t    expire; // when to expire 
    uint32_t    interval; // <=0 denotes oneshot
};

struct timeout_comp{
    bool operator()(const timeout &lhs, const timeout &rhs) const {
        return lhs.expire > rhs.expire;
    }
};

class timer{
public:
    timer() : timeoutq(timeout_comp()){
        timerfd=timerfd_open();
    }
    ~timer(){
        close(timerfd);
    }
    void reset_timerfd();
    int get_timerfd(){return timerfd;}
    // must be called by user threads, let reactor run push(t) in its eventloop 
    // Why don't we just use a mtx to protect the timer against race conditions? 
    // One reason is that mtx could potentially delay the execution of on_timerfd_event, which might be crucial if time accuracy matters.
    // Another reason is that push operations will be guranteed to finish before next epoll_wait call, hence even though it could be slower than concurrent modification with mtx, it's more stable.
    void please_push(timeout t);
    // must be called by user threads 
    void please_push(timer_cb cb, uint64_t expire, uint32_t interval=0);
    // must be called by reactor in its eventloop
    void on_timerfd_event();
    void push(const timeout& t); //worst: O(logn)
    void push(const timer_cb& cb, uint64_t expire, uint32_t interval=0);
    bool empty() const{
        return timeoutq.empty();
    }
    int size() const{
        return timeoutq.size();
    }
    timeout pop(); // worst: O(logn)
    void handle_expired_timeouts();
private:
    std::priority_queue<timeout, std::vector<timeout>, timeout_comp> timeoutq; 
    int timerfd;
};


}}