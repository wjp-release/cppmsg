#include "timer.h"
#include "reactor.h"

namespace msg{ namespace posix{

void timer::reset_timerfd(){
    timerfd_reset(timerfd, timeoutq.top().expire);
}

void timer::push(const timeout& t){ //worst: O(logn)
    timeoutq.push(t);
    if(t.expire==timeoutq.top().expire){ //auto reset
        reset_timerfd(); 
    }
}

void timer::push(const std::function<void(void)>& cb, uint64_t expire, uint32_t interval){
    timeoutq.emplace(cb, expire, interval);
    if(expire==timeoutq.top().expire){
        reset_timerfd();
    }
}

timeout timer::pop(){ // worst: O(logn)
    if(timeoutq.empty()) throw std::runtime_error("timeoutq is empty");
    timeout p = timeoutq.top();
    timeoutq.pop();
    return p;
}

void timer::on_timerfd_event(){  
    timerfd_read(timerfd);
    handle_expired_timeouts();
    std::cout<<"timerfd_event handled!\n\n";
}

void timer::handle_expired_timeouts(){
    if(timeoutq.empty()) return;
    while(!timeoutq.empty()){
        uint64_t ckpt=now();
        if(ckpt<timeoutq.top().expire) break;
        timeout t=pop();
        t.cb(); 
        if(t.is_periodic()){ 
            t.refresh(ckpt);
            timeoutq.push(t); // note that we don't use push here to avoid additional resets
        }
    }
    reset_timerfd(); // set next timerfd event's expire
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