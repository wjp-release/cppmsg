#pragma once

#include <queue>
#include <stdint.h>
#include <functional>
#include <vector>

namespace msg{ namespace posix{

struct timeout{
    timeout(const std::function<void(void)>& cb, uint64_t expire):cb(cb), expire(expire){}
    std::function<void(void)>   cb;
    uint64_t                    expire;
};

struct timeout_comp{
    bool operator()(const timeout &lhs, const timeout &rhs) const {
        return lhs.expire > rhs.expire;
    }
};

class timer{
public:
    timer() : timeoutq(timeout_comp()){}
    void push(const timeout& t){
        timeoutq.push(t);
    }
    void push(const std::function<void(void)>& cb, uint64_t expire){
        timeoutq.emplace(cb, expire);
    }
    timeout pop(){
        if(timeoutq.empty()) throw std::runtime_error("timeoutq is empty");
        timeout p = timeoutq.top();
        timeoutq.pop();
        return p;
    }
    bool empty() const{
        return timeoutq.empty();
    }
    int size() const{
        return timeoutq.size();
    }
private:
    std::priority_queue<timeout, std::vector<timeout>, timeout_comp> timeoutq;
};



}}