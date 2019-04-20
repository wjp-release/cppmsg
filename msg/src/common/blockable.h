#pragma once
#include <mutex>
#include <condition_variable>
#include "def.h"

namespace msg{namespace common{

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
};

}}
