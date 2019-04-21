#pragma once

#include "def.h"
#include "transport/conn.h"
#include "message.h"
#include "transport/iotask.h"
#include "common/blockable.h"
namespace msg{ namespace protocol{

class connection{
    using native_conn=std::unique_ptr<msg::transport::conn>;
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

// msg protocol level connection, owned by endpoints
class message_connection : public connection {
public:
    message_connection(int fd):connection(fd){}
    virtual ~message_connection(){}
    //Note that recvmsg(and the tasks it creates) cannot exlpoit the strength of readv to recv all msg parts in one task since we cannot know the size of msg body before reading the msg header.  
    //Too fully exploit the strength of readv/writev, you need to use the tranport layer zero-copy I/O interface directly.
    class recv_msghdr_task : public transport::oneiov_read_task, public common::blockable{
    private:
        uint64_t hdr=0;
        message_connection& c; 
        message& msg;
    public:
        recv_msghdr_task(message_connection& c, message& msg);
        virtual void on_success(int bytes);
        virtual void on_failure(int err);
    };

    class recv_msgbody_task : public transport::oneiov_read_task{
    private:
        std::shared_ptr<common::blockable> user_task;
    public:
        recv_msgbody_task(int size, message& msg, std::shared_ptr<common::blockable> user_task);
        virtual ~recv_msgbody_task(){}
        virtual void on_success(int bytes);
        virtual void on_failure(int err);
    };

    class send_msg_task : public transport::vector_write_task, public common::blockable{
    public:
        send_msg_task(const message& msg);
        virtual ~send_msg_task(){}
        virtual void on_success(int bytes);
        virtual void on_failure(int err);
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

// Success of sendmsg is defined as receving an ack from the receiver. 
// sendmsg tries to recv an ack after message_connection::sendmsg.
// recvmsg sends back an ack on a successful message_connection::recvmsg.
class reliable_connection : public message_connection{
public:

private:

};

/*
    Application level checksum:  
    In practice, the checksum is being asked to detect an error every few thousand packets. After eliminating those errors that the checksum always catches, the data suggests that, on average, between one packet in 10 billion and one packet in a few millions will have an error that goes undetected. The exact range depends on the type of data transferred and the path being traversed. While these odds seem large, they do not encourage complacency. In every trace, one or two 'bad apple' hosts or paths are responsible for a huge proportion of the errors. For applications which stumble across one of the `bad-apple' hosts, the expected time until a corrupted data is accepted could be as low as a few minutes. When compared to undetected error rates for local I/O (e.g., disk drives), these rates are disturbing. Our conclusion is that vital applications should strongly consider augmenting the TCP checksum with an application sum.
*/

// recvmsg sends back an ack after checksum the msg.
// sendmsg sends a msg with checksum info.
class very_reliable_connection : public message_connection{

};

// zero copy interface
class direct_connection : public connection{

};


}}