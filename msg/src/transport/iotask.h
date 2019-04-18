#pragma once

#include "def.h"
#include "common/taskpool.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include "task_interface.h"

/*
    struct iovec {
        void  *iov_base; // Starting address 
        size_t iov_len; // Number of bytes to transfer 
    };
*/

namespace msg{namespace transport{

// Zero application-level copy could be achieved by setting addresses in read/write tasks without copying data into a buffer. 
// The read_tasks or write_tasks are not responsible for releasing data. User must implement their own subclasses of read_task/write_task with overloaded on_success/on_failure callback functions.

struct io_task : public task_interface{
    virtual ~io_task(){}
    virtual iovec* iov()=0;
    virtual int iovcnt()=0;
};

struct read_task : public io_task{
    virtual ~read_task(){}
    bool try_scatter_input(int fd);
};

struct write_task : public io_task{
    virtual ~write_task(){}
    bool try_gather_output(int fd);
};

struct vector_io_task : public io_task{
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

struct vector_write_task : public vector_io_task, write_task{
    vector_write_task(int nriov, int msglen): vector_io_task(nriov, msglen){}
    virtual ~vector_write_task(){}
};

struct header_read_task : public vector_io_task, read_task{
    header_read_task(int nriov): vector_io_task(nriov){}
    virtual ~header_read_task(){}
};

struct oneiov_read_task : public read_task{
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


// Sometimes you do need a buffer, as its much more convenient than writing on_success/on_failure callbacks for the zero-copy approach. The following subclasses are provided for this purpose.

// subclass of read_task that has a buffer within itself
// stdlibc++ new/delete as malloc 

// struct heap_storage_read_task : public read_task{
//     virtual void on_success(int bytes){}
//     virtual void on_failure(int err){}
//     heap_storage_read_task(int size):read_task(1)
//     {
//         buf=new uint8_t[size];
//         iovs[1].iov_base=reinterpret_cast<void*>(buf);
//         iovs[1].iov_len=size;
//     }
//     ~heap_storage_read_task(){
//         delete [] buf;
//     }
//     uint8_t* buf;
// };

// // subclass of read_task that has a buffer within itself
// // fix-sized buffer
// template <int size>
// struct inline_storage_read_task : public read_task{
//     virtual void on_success(int bytes){}
//     virtual void on_failure(int err){}
//     inline_storage_read_task<size>():read_task(1){
//         iovs[1].iov_base=reinterpret_cast<void*>(buf);
//         iovs[1].iov_len=size;
//     }
//     uint8_t buf[size];
// };

// // most commonly used; one data copy
// struct one_part_write_task : public write_task{
//     virtual void on_success(int bytes){}
//     virtual void on_failure(int err){}
//     one_part_write_task(uint8_t* data, int msglen):write_task(1,msglen)
//     {
//         buf=new uint8_t[msglen];
//         iovs[1].iov_base=reinterpret_cast<void*>(buf);
//         iovs[1].iov_len=msglen;
//         memcpy(buf, data, msglen); //store data
//     }
//     ~one_part_write_task(){
//         delete [] buf;
//     }
//     uint8_t* buf;
// };



}}