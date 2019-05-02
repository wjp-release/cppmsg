#pragma once

#include "def.h"
#include "system/event.h"
#include "accept_task.h"
#include "system/addr.h"

namespace msg{

// A listener processes accept_tasks that accpet inbound connections.
class listener{
public:
    // create socket, bind, listen, and create event
    listener(const addr& a);
    ~listener(){ if(!closed) close();}
    void                    close();
    void                    start();
protected:
    void                    listener_cb(int evflag);
    void                    resubmit_accept();
    void                    accept();
private:
    event*                  e; 
    bool                    closed = false; 
    std::mutex              mtx;
    int                     newfd = -1;
};


}