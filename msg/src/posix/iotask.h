#pragma once

#include "def.h"
#include "taskpool.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>

/*
    struct iovec {
        void  *iov_base; // Starting address 
        size_t iov_len; // Number of bytes to transfer 
    };
*/

namespace msg{namespace posix{

enum task_failure_type : uint8_t{
    peer_closed = 1,
    fatal = 2, 
    conn_closed =3,
    other =4,
};

// Zero application-level copy could be achieved by setting addresses in read/write tasks without copying data into a buffer. 
// The read_tasks or write_tasks are not responsible for releasing data. User must implement their own subclasses of read_task/write_task with overloaded on_success/on_failure callback functions.

struct io_task{
    io_task(int nr_addrs, uint64_t header=0):iovs(nr_addrs+1), header(header){
        iovs[0].iov_base=reinterpret_cast<void*>(&header);
        iovs[0].iov_len=8;
    }
    virtual ~io_task(){}
    virtual void    on_success(int bytes)=0;
    virtual void    on_failure(int err)=0;
    std::vector<iovec> iovs;
    uint64_t        header; //msglen
};

struct read_task : public io_task{
    read_task(int nr_addrs): io_task(nr_addrs){}
    virtual ~read_task(){}
    bool            try_scatter_input(int fd);
};

struct write_task : public io_task{
    write_task(int nr_addrs, int msglen): io_task(nr_addrs, msglen){}
    virtual ~write_task(){}
    bool            try_gather_output(int fd);
};

// Sometimes you do need a buffer, as its much more convenient than writing on_success/on_failure callbacks for the zero-copy approach. The following subclasses are provided for this purpose.

// subclass of read_task that has a buffer within itself
// stdlibc++ new/delete as malloc 
struct heap_storage_read_task : public read_task{
    virtual void on_success(int bytes){}
    virtual void on_failure(int err){}
    heap_storage_read_task(int size):read_task(1)
    {
        buf=new uint8_t[size];
        iovs[1].iov_base=reinterpret_cast<void*>(buf);
        iovs[1].iov_len=size;
    }
    ~heap_storage_read_task(){
        delete [] buf;
    }
    uint8_t* buf;
};

// subclass of read_task that has a buffer within itself
// fix-sized buffer
template <int size>
struct inline_storage_read_task : public read_task{
    virtual void on_success(int bytes){}
    virtual void on_failure(int err){}
    inline_storage_read_task<size>():read_task(1){
        iovs[1].iov_base=reinterpret_cast<void*>(buf);
        iovs[1].iov_len=size;
    }
    uint8_t buf[size];
};

// most commonly used; one data copy
struct one_part_write_task : public write_task{
    virtual void on_success(int bytes){}
    virtual void on_failure(int err){}
    one_part_write_task(uint8_t* data, int msglen):write_task(1,msglen)
    {
        buf=new uint8_t[msglen];
        iovs[1].iov_base=reinterpret_cast<void*>(buf);
        iovs[1].iov_len=msglen;
        memcpy(buf, data, msglen); //store data
    }
    ~one_part_write_task(){
        delete [] buf;
    }
    uint8_t* buf;
};



}}