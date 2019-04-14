#pragma once

#include "def.h"
#include "reactor/event.h"
#include "transport/iotask.h"

namespace msg{namespace transport{

// A conn is an abstraction of an established 2-way connection, which processes both read_tasks and write_tasks.
class conn{
public:
    using write_sp=std::shared_ptr<write_task>;
    using read_sp=std::shared_ptr<read_task>;
    // note that fd is an established connection's fd
    conn(int fd);
    ~conn(){ if(!closed) close();}
    void                 add_read(const read_sp& m);
    void                 add_write(const write_sp& m);
    void                 close();
protected:
    void                 conn_cb(int evflag);
    void                 resubmit_both();
    void                 resubmit_read();
    void                 resubmit_write();
    void                 read();
    void                 write();
private:
    msg::reactor::event* e; 
    bool                 closed = false; 
    std::list<write_sp>  writes;
    std::list<read_sp>   reads; 
    std::mutex           mtx;
};

}} 