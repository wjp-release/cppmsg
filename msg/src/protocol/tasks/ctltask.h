#pragma once

#include "def.h"
#include "task_interface.h"
#include "system/addr.h"
#include "system/fd.h"

namespace msg{

struct ctl_task: public task_interface{
    ctl_task(){}
    virtual ~ctl_task(){}
    int newfd=-1;  // reserved for future connection
};

enum accept_result : uint8_t{
    damn_we_are_good = 0,  // trigger on_success(0), remove from list
    ignore_this_continue = 1, // continue processing other accept tasks
    should_stop_all =2, // break current run, come back next round
    fatal_err_just_drop_it =3, // trigger on_failure(fatal), remove from list
};

// Why accept4? 
// The reason for SOCK_CLOEXEC to exist is to avoid a race condition between getting a new socket from accept and setting the FD_CLOEXEC flag afterwards.

// Why EWOULDBLOCK?
// The socket is marked nonblocking and no connections are present to be accepted. POSIX.1-2001 allows either EAGAIN or EWOULDBLOCK to be returned for this case, and does not require these constants to have the same value, so a portable application should check for both possibilities.

// Which errno should be treated like EAGAIN?
// Quoted from https://linux.die.net/man/2/accept4:
// For reliable operation the application should detect the network errors defined for the protocol after accept() and treat them like EAGAIN by retrying. In the case of TCP/IP, these are ENETDOWN, EPROTO, ENOPROTOOPT, EHOSTDOWN, ENONET, EHOSTUNREACH, EOPNOTSUPP, and ENETUNREACH.

struct accept_task : public ctl_task{
    accept_task(){}
    virtual ~accept_task(){}
    accept_result try_accept(int fd);
};

struct connect_task : public ctl_task{
    connect_task(){}
    virtual ~connect_task(){}
    bool try_connect();
    void handle_async_connect_result();
    addr a;
};


    


}