#pragma once

#include "def.h"
#include "transport/conn.h"
#include "message.h"
#include "transport/iotask.h"
#include "common/blockable.h"

namespace msg{ namespace protocol{
class endpoint;
class connection;

//Note that recvmsg(and the tasks it creates) cannot exlpoit the strength of readv to recv all msg parts in one task since we cannot know the size of msg body before reading the msg header.  
//Too fully exploit the strength of readv/writev, you need to use the tranport layer zero-copy I/O interface directly.
class recv_msghdr_task : public transport::oneiov_read_task, public common::blockable{
private:
    uint64_t hdr=0;
    connection& c; 
    message& msg;
public:
    recv_msghdr_task(connection& c, message& msg) : transport::oneiov_read_task(reinterpret_cast<void*>(&hdr), 8), c(c), msg(msg){}
    virtual void on_success(int bytes);
    virtual void on_failure(int err);
};

class recv_msgbody_task : public transport::oneiov_read_task{
private:
    std::shared_ptr<common::blockable> user_task;
public:
    recv_msgbody_task(int size, message& msg, std::shared_ptr<common::blockable> user_task): 
    transport::oneiov_read_task(msg.alloc(size), size),
    user_task(user_task){}
    virtual ~recv_msgbody_task(){}
    // Now we have filled the msg, wake up user.
    virtual void on_success(int bytes);
    virtual void on_failure(int err);
};

class send_msg_task : public transport::vector_write_task, public common::blockable{
public:
    virtual ~send_msg_task(){}
    send_msg_task(const message& msg) : transport::vector_write_task(msg.nr_chunks()+1, msg.size()){
        msg.append_iov(iovs); 
    }
    virtual void on_success(int bytes);
    virtual void on_failure(int err);
};

// msg protocol level connection, owned by endpoints
class connection {
public:
    using native_conn=std::unique_ptr<msg::transport::conn>;
    connection(endpoint& ep, int fd):owner(ep), conn(std::make_unique<msg::transport::conn>(fd)){

    }
    ~connection(){
        close();
    }
    // blocking method
    void            sendmsg(const message& msg){
        std::unique_lock<std::mutex> lk(mtx);
        auto task=std::make_shared<send_msg_task>(msg);
        conn->add_write(task);
        task->wait();
    }
    // blocking method
    void            recvmsg(message& msg){
        std::unique_lock<std::mutex> lk(mtx);
        auto task=std::make_shared<recv_msghdr_task>(*this, msg);
        conn->add_read(task);
        task->wait();
    }
    // nonblocking method, cb will be executed in threadpool
    void            sendmsg_async(const message& msg, const async_cb& cb){
        //todo 
    }
    // zero copy
    void            recv_direct(){
        //todo
    }
    // close of a connection will trigger on_failure(conn_close) of all its io_tasks
    void            close(){
        conn->close();
    }
    native_conn     conn;  // thread-safe
private:
    uint8_t         head_read_buffer[8]; // an 8-bytes read_task buffer for reading msg headers
    uint8_t         header_writer_buffer[8]; // an 8-bytes write_task buffer for writing msg headers
    endpoint&       owner; 
    std::mutex      mtx;
    std::condition_variable cv;
};


}}