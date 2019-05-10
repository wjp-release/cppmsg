#pragma once
#include "def.h"
#include <condition_variable>
#include <chrono>
using namespace std::chrono_literals;

namespace msg{

class blockable : public std::enable_shared_from_this<blockable>{
private:
    mutable std::mutex mtx;
    mutable std::condition_variable cv;
    bool is_done=false;
public:
    void signal(){ 
        {
            std::lock_guard<std::mutex> lk(mtx);
            is_done=true;
        }
        cv.notify_one();
    }
    void wait(){
        std::unique_lock<std::mutex> lk(mtx);
        while(!is_done){
            cv.wait(lk);
        }
    }
    bool wait_for(int milliseconds){
        std::unique_lock<std::mutex> lk(mtx);
        while(!is_done){
            cv.wait_for(lk, milliseconds*1ms);
        }
        return is_done;
    }
};

}
