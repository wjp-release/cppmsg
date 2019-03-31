#pragma once

#include <queue>
#include <stdint.h>
#include <functional>
#include <vector>

#include "clock.h"
#include "fd.h"

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
    std::function<void(void)>   cb; // cb finishes fast or runs in a background thread
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
protected:
    // Despite being thread-unsafe, protected functions will
    // only be run in eventloop sequentially. 
    void push(const timeout& t){ //worst: O(logn)
        timeoutq.push(t);
        if (timeoutq.top().expire==t.expire){ 
            timerfd_reset(timerfd, t.expire);
        }
    }
    void push(const std::function<void(void)>& cb, uint64_t expire, uint32_t interval=0){
        timeoutq.emplace(cb, expire, interval);
        if (timeoutq.top().expire==expire){ 
            timerfd_reset(timerfd, expire);
        }
    }
    bool empty() const{
        return timeoutq.empty();
    }
    int size() const{
        return timeoutq.size();
    }
    timeout pop(){ // worst: O(logn)
        if(timeoutq.empty()) throw std::runtime_error("timeoutq is empty");
        timeout p = timeoutq.top();
        timeoutq.pop();
        return p;
    }
    uint64_t earliest_expire(){
        return timeoutq.top().expire;
    }
    void handle_expired_timeouts(){
        uint64_t ckpt=now();
        while(ckpt>earliest_expire()){
            timeout t=pop();
            t.cb(); 
            if(t.is_periodic()){ 
                t.refresh(ckpt);
                push(t); 
            }
        }
    } 
private:
    std::priority_queue<timeout, std::vector<timeout>, timeout_comp> timeoutq;
    int timerfd;
};



}}