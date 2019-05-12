#pragma once

#include "def.h"
#include "connection.h"

namespace msg{ 


/*
    A basic connection provides basic implementation of connection-oriented message API.
*/

class basic_connection : public connection, public std::enable_shared_from_this<basic_connection> {
public:
    // Users must create basic_connection as shared_ptr.
    static std::shared_ptr<basic_connection> make(int fd){
        return std::make_shared<basic_connection>(fd);
    }
    basic_connection(int fd):connection(fd){
        backoff_routine=[this]{
            c->resubmit_write(); // backoff resubmit
        };
    }
    virtual ~basic_connection(){}
    void              set_send_tmo(int ms){
        send_timeout=ms;
    }
    void              set_recv_tmo(int ms){
        recv_timeout=ms;
    }
    void              enable_nagle(){
        nagle=true;
    }
    void              disable_nagle(){
        nagle=false;
    }
    virtual status    sendmsg(const message& msg);
    virtual void      sendmsg_async(const message& msg, const async_cb& cb=nullptr){
        //todo 
    }

    virtual status    recvmsg(message& msg);
    virtual void      recvmsg_async(message& msg, const async_cb& cb=nullptr){
        //todo 
    }

    virtual status    recv_multipart_msg(message& msg){


        return status::success();
    }

    virtual void      recv_multipart_msg_async(message& msg, const async_cb& cb=nullptr){
        


    }
    uint64_t          hdr(){
        return *reinterpret_cast<uint64_t*>(hdrbuf);
    }  
    timer_cb          backoff_cb(){
        return backoff_routine;
    }
private:
    class bodytask : public oneiov_read_task{ 
    private:
        std::shared_ptr<blockable> user_task;
    public:
        bodytask(int size, message& msg, std::shared_ptr<blockable> user_task, const connptr& c): 
            oneiov_read_task(msg.alloc(size), size, c),
            user_task(user_task)
        {}
        virtual ~bodytask(){}
        virtual void on_success(int bytes, std::unique_lock<std::mutex>& lk);
        virtual void on_recoverable_failure(int backoff);
    };

    class hdrtask : public oneiov_read_task, public blockable{
    private:
        message& msg; 
    public:
        std::shared_ptr<bodytask> subtask=nullptr;
        hdrtask(void*buf, message& msg, const connptr& c): 
        oneiov_read_task(buf, 8, c), msg(msg){}
        bool failure=false;
        virtual void on_success(int bytes, std::unique_lock<std::mutex>& lk);
        virtual void on_recoverable_failure(int backoff);
    };
    timer_cb          backoff_routine;
    char              hdrbuf[8]; // hdr buffer
    uint32_t          send_timeout=0; // ms
    uint32_t          recv_timeout=0; // ms
    bool              nagle=false;
};



}