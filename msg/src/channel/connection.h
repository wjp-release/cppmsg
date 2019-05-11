#pragma once

#include "def.h"
#include "pipe.h"
#include "message.h"
#include "iotask.h"
#include "common/blockable.h"
namespace msg{ 

class connection{
    using native_conn=std::unique_ptr<pipe>;
protected:
    native_conn     c;  // thread-safe
public:
    connection(int fd);
    virtual ~connection(){
        close();
    }
    // close of a connection will trigger on_failure(conn_close) of all its io_tasks
    void            close(){
        c->close();
    }
    // Send a msg synchronously, block until success. Note that success could be defined as syscall sucess, ack success, peer checksum success, etc.
    virtual status  sendmsg(const message& msg)=0;
    // Send a msg asynchronously, trigger cb on success.
    virtual void    sendmsg_async(const message& msg, const async_cb& cb=nullptr)=0;
    // Recv a single-chunk msg.
    virtual status  recvmsg(message& msg)=0;
    virtual void    recvmsg_async(message& msg, const async_cb& cb=nullptr)=0;
    // Recv multiple messages and group them into one multi-chunk message.
    virtual status  recv_multipart_msg(message& msg)=0;
    virtual void    recv_multipart_msg_async(message& msg,const async_cb& cb=nullptr)=0;  

};





// zero copy interface
class direct_connection : public connection{
public:
    direct_connection(int fd):connection(fd){}
    virtual ~direct_connection(){}
    virtual status    sendmsg(const message& msg){
        return status::success();  
    }
    virtual void    sendmsg_async(const message& msg, const async_cb& cb=nullptr){

    }
    virtual status    recvmsg(message& msg){
        return status::success();
    }
    virtual void    recvmsg_async(const message& msg, const async_cb& cb=nullptr){

    }
};


}