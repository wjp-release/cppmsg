#pragma once

#include "def.h"
#include "system/event.h"
#include "connect_task.h"
#include "common/taskpool.h"
#include "system/fd.h"
#include "addr.h"


namespace msg{

// A connector tries to connect on construction. 
class connector{
public:
    using connect_sp=std::shared_ptr<connect_task>;
    connector(const connect_sp& csp): ctask(csp){}
    ~connector(){ if(!closed) close(); }
    // return if immediately connected; if not, create an async connect event.
    status                  connect(); 
    void                    close();
    status                  create_event(int newfd);
protected:
    void                    connect_cb(int evflag);
private:
    connect_sp              ctask;
    event*                  e = nullptr; 
    bool                    closed = false; 
    std::mutex              mtx; // protect close
};


}}