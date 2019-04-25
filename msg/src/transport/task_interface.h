#pragma once

#include "def.h"

namespace msg{namespace transport{

struct task_interface{
    virtual void on_success(int duh)=0; // duh could be bytes written/read or a success code
    virtual void on_failure(int err)=0; // err 
};

enum task_success_duh : uint8_t{
    it_seems_good = 0, // We don't have extra info about this success.
    immediate_connect = 1, // You can release the connector now since it's already done!
    async_connect = 2, // You should probably release the connector via on_success callback.
};

enum task_failure_reason : uint8_t{
    peer_closed = 1, // A read() that returns 0 bytes indicates peer has closed.
    fatal = 2, // It's so wrong that we would rather drop this task and erase it from any pending list.
    conn_closed = 3, // Pending io_tasks in a conn will fail over this reason
    trivial =4, // It's safe to do nothing on failure 
    bad_but_recoverable = 5, // Could be a temporary resource exhaustion, stop, and come back next epoll loop.
    system_wide_fatal = 6, // It's so wrong that we would rather let the entire system down.
    listener_closed = 7,
};

static inline std::string reason(int err){
    switch(err){
    case peer_closed: 
        return "peer closed";
    case fatal:
        return "fatal; we should drop this task and erase it from any pending list";
    case conn_closed:
        return "conn closed";
    case trivial:
        return "trivial; it's safe to ignore it";
    case bad_but_recoverable:
        return "bad but recoverable, could be a temporary resource exhaustion; stop now and come back later";
    case system_wide_fatal:
        return "It's so wrong that we would let the entire system down!";
    }
    return "invalide err";
}


}}