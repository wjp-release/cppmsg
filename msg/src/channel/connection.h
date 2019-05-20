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