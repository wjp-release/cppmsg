#include "timer.h"
#include "reactor.h"

namespace msg{ namespace posix{

void timer::push(const timeout& t){ //worst: O(logn)
    timeoutq.push(t);
    if (timeoutq.top().expire==t.expire){ 
        timerfd_reset(timerfd, t.expire);
    }
}

void timer::push(const std::function<void(void)>& cb, uint64_t expire, uint32_t interval){
    timeoutq.emplace(cb, expire, interval);
    if (timeoutq.top().expire==expire){ 
        timerfd_reset(timerfd, expire);
    }
}

timeout timer::pop(){ // worst: O(logn)
    if(timeoutq.empty()) throw std::runtime_error("timeoutq is empty");
    timeout p = timeoutq.top();
    timeoutq.pop();
    return p;
}

void timer::handle_expired_timeouts(){
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

void timer::please_push(timeout t){ 
    reactor::instance().run([this,t]{
        push(t);
    });
}
// must be called by user threads 
void timer::please_push(func cb, uint64_t expire, uint32_t interval){
    reactor::instance().run([this, cb, expire, interval]{
        push(cb, expire, interval);
    });
}

}}