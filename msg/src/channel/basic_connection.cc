#include "basic_connection.h"
#include "system/reactor.h"
namespace msg{
    
status basic_connection::sendmsg(const message& msg){
    class sendtask : public vector_write_task, public blockable{
    public:
        timer_cb cb;
        sendtask(const message& msg, timer_cb cb) : vector_write_task(msg.nr_chunks()+1, msg.size()), cb(cb){
            msg.append_to_iovs(iovs); 
        }
        virtual ~sendtask(){}
        virtual void on_success(int bytes){
            logdebug("%d bytes written", bytes);
            signal(); 
        }
        virtual void on_recoverable_failure(int backoff){
            defer(cb, backoff);
        }
    };
    auto task=std::make_shared<sendtask>(msg, [this]{
        c->resubmit_write(); // backoff resubmit
    });
    c->add_write(task);
    if(task->wait_for(send_timeout)){
        return status::success();
    }else{
        c->remove_write(task);
        return status::failure("timeout");
    }            
}

status basic_connection::recvmsg(message& msg){
    // class hdrtask : public oneiov_read_task, public blockable{
    // private:
    //     uint64_t hdr=0;
    //     basic_connection& c; 
    //     message& msg;
    // public:
    //     hdrtask(basic_connection& c, message& msg);
    //     virtual void on_success(int bytes);
    //     virtual void on_recoverable_failure();
    // };
    // class bodytask : public oneiov_read_task{
    // private:
    //     std::shared_ptr<blockable> user_task;
    // public:
    //     bodytask(int size, message& msg, std::shared_ptr<blockable> user_task);
    //     virtual ~bodytask(){}
    //     virtual void on_success(int bytes);
    //     virtual void on_recoverable_failure();
    // };

    // auto task=std::make_shared<hdrtask>(*this, msg);
    // c->add_read(task);
    // task->wait();
}



}