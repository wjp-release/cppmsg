#pragma once

#include "def.h"
#include "conn.h"
#include "message.h"
#include "iotask.h"
#include "common/blockable.h"
namespace msg{ 

class connection{
    using native_conn=std::unique_ptr<conn>;
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
    virtual void    sendmsg(const message& msg)=0;
    virtual void    sendmsg_async(const message& msg, const async_cb& cb=nullptr)=0;
    virtual void    recvmsg(message& msg)=0;
    virtual void    recvmsg_async(const message& msg, const async_cb& cb=nullptr)=0;
};





// zero copy interface
class direct_connection : public connection{
public:
    direct_connection(int fd):connection(fd){}
    virtual ~direct_connection(){}
    virtual void    sendmsg(const message& msg){
        
    }
    virtual void    sendmsg_async(const message& msg, const async_cb& cb=nullptr){

    }
    virtual void    recvmsg(message& msg){

    }
    virtual void    recvmsg_async(const message& msg, const async_cb& cb=nullptr){

    }
};


}