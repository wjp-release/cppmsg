#pragma once

#include "def.h"
#include "transport/conn.h"
#include "message.h"
#include "transport/iotask.h"
namespace msg{ namespace protocol{
class endpoint;

class blockable{
private:
    mutable std::mutex mtx;
    mutable std::condition_variable cv;
    bool is_done=false;
public:
    void signal(){ 
        {
            std::lock_guard<std::mutex> lk(mtx);
            is_done=true;
        }
        cv.notify_one();
    }
    void wait(){
        std::unique_lock<std::mutex> lk(mtx);
        while(!is_done){
            cv.wait(lk);
        }
    }
};

//Note that recvmsg(and the tasks it creates) cannot exlpoit the strength of readv to recv all msg parts in one task since we cannot know the size of msg body before reading the msg header.  
//Too fully exploit the strength of readv/writev, you need to use the tranport layer zero-copy I/O interface directly.
class recv_msghdr_task : public oneiov_read_task, blockable, std::enable_shared_from_this<recv_msghdr_task>{
private:
    uint64_t hdr=0;
    connection& c; 
    message& msg;
public:
    recv_msghdr_task(connection& c, message& msg) : oneiov_read_task(reinterpret_cast<void*>(&hdr), 8), c(c), msg(msg){}
    // msghdr contains msglen, so we can create recv_msgbody_task
    virtual void on_success(int bytes){
        std::cout<<"hdr: msglen="<<hdr<<std::endl;
        c.conn.add_read(std::make_shared<recv_msgbody_task>(hdr, msg, shared_from_this()));
    }
    virtual void on_failure(int err){
        if(err==peer_closed){
            //todo: replace iostream with a logger
            std::cerr<<"recv_msghdr_task peer closed"<<std::endl;
        }
    }
};

class recv_msgbody_task : public oneiov_read_task{
private:
    std::shared_ptr<recv_msghdr_task> user_task;
public:
    recv_msgbody_task(int size, message& msg, std::shared_ptr<recv_msghdr_task> user_task):user_task(user_task){
        v.iov_base=msg.alloc(size);
        v.iov_len=size;
    }
    // Now we have filled the msg, wake up user.
    virtual void on_success(int bytes){
        user_task->signal(); 
    }
    virtual void on_failure(int err){
        if(err==peer_closed){
            std::cerr<<"recv_msgbody_task peer closed"<<std::endl;
        }
    }
};

class send_msg_task : public vector_write_task, blockable{
public:
    send_msg_task(const message& msg) : vector_io_task(msg.nr_chunks()+1, msg.size()){
        msg.append_iov(iovs); 
    }
    // Now we have filled the msg, wake up user.
    virtual void on_success(int bytes){
        user_task->signal(); 
    }
    virtual void on_failure(int err){
        if(err==peer_closed){
            std::cerr<<"recv_msgbody_task peer closed"<<std::endl;
        }
    }
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
        auto task=std::make_shared<send_msg_task>(*this, msg);
        conn.add_write(task);
        task->wait();
    }
    // blocking method
    void            recvmsg(message& msg){
        std::unique_lock<std::mutex> lk(mtx);
        auto task=std::make_shared<recv_msghdr_task>(*this, msg);
        conn.add_read(task);
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

private:
    native_conn     conn;  // thread-safe
    uint8_t         head_read_buffer[8]; // an 8-bytes read_task buffer for reading msg headers
    uint8_t         header_writer_buffer[8]; // an 8-bytes write_task buffer for writing msg headers
    endpoint&       owner; 
    std::mutex      mtx;
    std::condition_variable cv;
};


}}