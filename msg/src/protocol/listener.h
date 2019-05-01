#pragma once

#include "def.h"
#include "system/event.h"
#include "ctltask.h"
#include "system/addr.h"

namespace msg{

// A listener processes accept_tasks that accpet inbound connections.
class listener{
public:
    using accept_sp=std::shared_ptr<accept_task>;
    // create socket, bind, listen, and create event
    listener(const addr& a);
    ~listener(){ if(!closed) close();}
    void                    close();
    void                    add_accept(const accept_sp& a);
protected:
    void                    listener_cb(int evflag);
    void                    resubmit_accept();
    void                    accept();
private:
    msg::reactor::event*    e; 
    std::list<accept_sp>    accepts;
    bool                    closed = false; 
    std::mutex              mtx;
};


}