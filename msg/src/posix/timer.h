#pragma once

#include "def.h"
#include <queue>
#include <stdint.h>
#include <functional>
#include <vector>

#include "clock.h"
#include "fd.h"
#include <unistd.h> // close

// timeout: encapsulation of when, and what to do 
// timer: a compact and fast timerfd-based timer

namespace msg{ namespace posix{

struct timeout{
    timeout(const std::function<void(void)>& cb, uint64_t expire, uint32_t interval):cb(cb), expire(expire), interval(interval){}
    void                        refresh(uint64_t baseline){
        expire=baseline+interval;
    }
    bool                        is_periodic(){
        return interval>0;
    }
    std::function<void(void)>   cb; // cb must finish very fast or run in a background thread
    uint64_t                    expire; // when to expire 
    uint32_t                    interval; // <=0 denotes oneshot
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
    // must be called by user threads, let reactor run push(t) in its eventloop 
    void please_push(const timeout& t){ 
        
    }
    // must be called by user threads 
    void please_push(const std::function<void(void)>& cb, uint64_t expire, uint32_t interval=0){

    }
    // must be called by reactor in its eventloop
    void on_timerfd_event(){  
        timerfd_read(timerfd);
        handle_expired_timeouts();
    }
    /*
        Despite being thread-unsafe, following functions will only be run in eventloop sequentially. 
    */
    void push(const timeout& t); //worst: O(logn)
    void push(const std::function<void(void)>& cb, uint64_t expire, uint32_t interval=0);
    bool empty() const{
        return timeoutq.empty();
    }
    int size() const{
        return timeoutq.size();
    }
    timeout pop(); // worst: O(logn)
    uint64_t earliest_expire(){
        return timeoutq.top().expire;
    }
    void handle_expired_timeouts();
private:
    std::priority_queue<timeout, std::vector<timeout>, timeout_comp> timeoutq;
    int timerfd;
};


}}