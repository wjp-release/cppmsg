#pragma once

#include "def.h"

namespace msg{namespace posix{namespace tcp{

class msgbuf{
public:
    msgbuf(){}
    void        on_transferred(){
        if(transferred_cb) transferred_cb();
    }
    void        on_failure(){
        if(failure_cb) failure_cb();
    }

private:
    task_cb     transferred_cb;
    task_cb     failure_cb;
};



}}}