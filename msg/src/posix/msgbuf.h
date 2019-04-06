#pragma once

#include "def.h"
#include "taskpool.h"
#include <vector>

namespace msg{namespace posix{

enum msg_transfer_failure : uint8_t{
    peer_closed = 1,
    other = 2, 
    conn_closed =3,
};

namespace detail{ class msgbuf_impl; }

// It's recommended to apply value semantics for msgbuf objects. 
class msgbuf{
public:
    msgbuf(){} // default ctor, mptr==nullptr   
    ~msgbuf(){}                      
    msgbuf(const transferred_cb& tcb, const failure_cb& fcb, int nr_iov);
    msgbuf(const msgbuf& m) : mptr(m.mptr){}
    msgbuf& operator=(const msgbuf& other) noexcept;
    void    on_transferred();
    void    on_failure(int what);
    bool    try_scatter_input(int fd);
    bool    try_gather_output(int fd);
    void    add_iov(void* addr, size_t len);
private:
    std::shared_ptr<detail::msgbuf_impl> mptr=nullptr;
};

}}