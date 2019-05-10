#pragma once

#include "def.h"
#include "system/event.h"
#include "iotask.h"

namespace msg{

// A pipe is an abstraction of an established 2-way physical connection channel, which processes both read_tasks and write_tasks.
class pipe{
public:
    using write_sp=std::shared_ptr<write_task>;
    using read_sp=std::shared_ptr<read_task>;
    // note that fd is an established tcp connection's fd
    pipe(int fd); // may fail: epoll_ctl add 
    ~pipe(){ if(!closed) close();}
    // public functions acquire mtx ownership
    void                 add_read(const read_sp& m);
    void                 add_write(const write_sp& m);
    void                 remove_read(const read_sp& m);
    void                 remove_write(const write_sp& m);
    void                 clear_reads();
    void                 clear_writes();
    void                 close(); 
    void                 resubmit_both();
    void                 resubmit_read();
    void                 resubmit_write();
    void                 read(); 
    void                 write(); 
    void                 pipe_cb(int evflag); 
    uint16_t             get_backoff() const noexcept;
protected:
    void                 dosubmit_both();
    void                 dosubmit_read();
    void                 dosubmit_write();
    void                 doclose();
    void                 doread(std::unique_lock<std::mutex>&);
    void                 dowrite(std::unique_lock<std::mutex>&);
private:
    void                 adjust_backoff();
    event*               e; 
    bool                 closed = false; 
    std::list<write_sp>  writes;
    std::list<read_sp>   reads; 
    mutable std::mutex   mtx;
    uint16_t             backoff;

};

}