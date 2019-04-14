#pragma once

#include "def.h"
#include "reactor/event.h"
#include "transport/ctltask.h"
#include "common/taskpool.h"
#include "common/fd.h"
#include "addr.h"


namespace msg{namespace transport{

// A connector tries to connect on construction. 
class connector{
public:
    using connect_sp=std::shared_ptr<connect_task>;
    connector(const connect_sp& csp): ctask(csp){}
    ~connector(){ if(!closed) close(); }
    // return if immediately connected; if not, create an async connect event.
    bool                    connect(); 
    void                    close();
protected:
    void                    connect_cb(int evflag);
private:
    connect_sp              ctask;
    msg::reactor::event*    e = nullptr; 
    bool                    closed = false; 
    std::mutex              mtx; // protect close
};


}}