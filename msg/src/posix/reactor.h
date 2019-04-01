#pragma once

#include "def.h"

namespace msg{ namespace posix{

class reactor{
public:
    static reactor& instance(){
        static reactor r;
        return r;
    }
    int epollfd;
    ~reactor(){

    }
    // wake event loop, run it in the loop immediately
    void run(const func& cb);
    // not very urgent, queue it, run in next loop
    void run_later(const func& cb);
protected:
    reactor(){

    }
private:

};


}}