#pragma once

#include "def.h"
#include "connection.h"

namespace msg{ 

// msg protocol level connection, owned by endpoints
class basic_connection : public connection {
public:
    basic_connection(int fd):connection(fd){}
    virtual ~basic_connection(){}
    //Note that recvmsg(and the tasks it creates) cannot exlpoit the strength of readv to recv all msg parts in one task since we cannot know the size of msg body before reading the msg header.  
    //Too fully exploit the strength of readv/writev, you need to use the tranport layer zero-copy I/O interface directly.
    class recv_msghdr_task : public oneiov_read_task, public blockable{
    private:
        uint64_t hdr=0;
        basic_connection& c; 
        message& msg;
    public:
        recv_msghdr_task(basic_connection& c, message& msg);
        virtual void on_success(int bytes);
        virtual void on_recoverable_failure();
    };

    class recv_msgbody_task : public oneiov_read_task{
    private:
        std::shared_ptr<blockable> user_task;
    public:
        recv_msgbody_task(int size, message& msg, std::shared_ptr<blockable> user_task);
        virtual ~recv_msgbody_task(){}
        virtual void on_success(int bytes);
        virtual void on_recoverable_failure();
    };

    class send_msg_task : public vector_write_task, public blockable{
    public:
        send_msg_task(const message& msg);
        virtual ~send_msg_task(){}
        virtual void on_success(int bytes);
        virtual void on_recoverable_failure();
    };

    // block until writev succeeds
    virtual void    sendmsg(const message& msg);
    // return immediately; cb will be executed in threadpool on writev success
    virtual void    sendmsg_async(const message& msg, const async_cb& cb=nullptr){
        //todo 
    }

    // block until readv succeeds
    virtual void    recvmsg(message& msg);
    // return immediately; cb will be executed in threadpool on readv success
    virtual void    recvmsg_async(const message& msg, const async_cb& cb=nullptr){
        //todo 
    }
private:
    // todo: options like open/close nagle, send/recv timeouts, etc
    // todo: heartbeats 
};



}