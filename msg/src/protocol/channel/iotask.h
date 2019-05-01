#pragma once

#include "def.h"
#include "common/taskpool.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

/*
    struct iovec {
        void  *iov_base; // Starting address 
        size_t iov_len; // Number of bytes to transfer 
    };
*/

namespace msg{

// Zero application-level copy could be achieved by setting addresses in read/write tasks without copying data into a buffer. 
// The read_tasks or write_tasks are not responsible for releasing data. User must implement their own subclasses of read_task/write_task with overloaded on_success/on_failure callback functions.

class io_task{
public:
    virtual ~io_task(){}
    virtual void on_success(int bytes_transferred)=0;
    virtual void on_recoverable_failure()=0;
    virtual void on_peer_closed(){};
    virtual void on_conn_closed(){};
    virtual iovec* iov()=0;
    virtual int iovcnt()=0;
};

class read_task : public io_task{
public:
    virtual ~read_task(){}
    bool try_scatter_input(int fd);
};

class write_task : public io_task{
public:
    virtual ~write_task(){}
    bool try_gather_output(int fd);
};

class vector_io_task : public io_task{
public:
    vector_io_task(int nriov, int msglen=0): iovs(nriov), header(msglen){
        iovs[0].iov_base=reinterpret_cast<void*>(&msglen);
        iovs[0].iov_len=8;
    }
    virtual iovec* iov(){ 
        return iovs.data();
    }
    virtual int iovcnt(){ 
        return iovs.size();
    }
    virtual ~vector_io_task(){}
    std::vector<iovec> iovs;
    uint64_t header; //msglen
};

class vector_write_task : public vector_io_task, public write_task{
public:
    vector_write_task(int nriov, int msglen): vector_io_task(nriov, msglen){}
    virtual ~vector_write_task(){}
    virtual iovec* iov(){ 
        return vector_io_task::iov();
    }
    virtual int iovcnt(){ 
        return vector_io_task::iovcnt();
    }
};

class header_read_task : public vector_io_task, public read_task{
public:
    header_read_task(int nriov): vector_io_task(nriov){}
    virtual ~header_read_task(){}
    virtual iovec* iov(){ 
        return vector_io_task::iov();
    }
    virtual int iovcnt(){ 
        return vector_io_task::iovcnt();
    }
};

class oneiov_read_task : public read_task{
public:
    oneiov_read_task(void* base, int size){
        v.iov_base=base;
        v.iov_len=size;
    }
    virtual ~oneiov_read_task(){}
    virtual iovec* iov(){ 
        return &v;
    }
    virtual int iovcnt(){ 
        return 1;
    }
    iovec v;
};



}