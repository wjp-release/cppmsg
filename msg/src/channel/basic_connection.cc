#include "basic_connection.h"
#include "system/reactor.h"
namespace msg{

status basic_connection::sendmsg(const message& msg){
    class sendtask : public vector_write_task, public blockable{
    public:
        sendtask(const message& msg, const connptr& c) : vector_write_task(msg.nr_chunks()+1, msg.size(), c){
            msg.append_to_iovs(iovs); 
        }
        virtual ~sendtask(){}
        virtual void on_success(int bytes, std::unique_lock<std::mutex>& lk){
            logdebug("%d bytes written", bytes);
            signal(); 
        }
        virtual void on_recoverable_failure(int backoff){
            auto c=conn.lock();
            if(!c){ 
                logerr("This task's owning connection has been destroyed, hence this task should have been discarded. Therefore we won't do anything to recover it.");
            }else{
                defer(c->backoff_cb(), backoff);
            }
        }
    };
    try{
        auto task=std::make_shared<sendtask>(msg, std::weak_ptr<basic_connection>(shared_from_this()));
        c->add_write(task);
        if(task->wait_for(send_timeout)){
            return status::success();
        }else{
            c->remove_write(task);
            return status::failure("timeout");
        }            
    }catch(const std::exception& e){ 
        return status::fault(e.what());
    }catch(...){
        return status::fault("unknown exception");
    }
}

// message hdr read success
void basic_connection::hdrtask::on_success(int bytes, std::unique_lock<std::mutex>& lk){
    auto c=conn.lock();
    if(!c){
        logerr("This task's owning connection has been destroyed, hence this task should have been discarded. Therefore we won't further process it.");
        return;
    }
    uint64_t hdr=c->hdr();
    logdebug("%d bytes read, hdr contains msglen, which is %d", bytes, hdr);
    if(hdr>message::MaxSize){
        failure=true;
        signal(); //obviously illegal, discard it 
        return;
    }
    subtask=std::make_shared<bodytask>(hdr, msg, shared_from_this(), conn);
    c->c->doadd_read(lk, subtask);
}

// message hdr read recoverable failure
void basic_connection::hdrtask::on_recoverable_failure(int backoff){
    auto c=conn.lock();
    if(!c){
        logerr("This task's owning connection has been destroyed, hence this task should have been discarded. Therefore we won't try to recover it.");
        return;
    }
    defer(c->backoff_routine, backoff);
}

// Now we have filled the msg, wake up user.
void basic_connection::bodytask::on_success(int bytes, std::unique_lock<std::mutex>& lk){
    logdebug("%d bytes read", bytes);
    user_task->signal(); 
}

void basic_connection::bodytask::on_recoverable_failure(int backoff){
    auto c=conn.lock();
    if(!c){
        logerr("This task's owning connection has been destroyed, hence this task should have been discarded. Therefore we won't try to recover it.");
        return;
    }
    defer(c->backoff_routine, backoff);
}

status basic_connection::recvmsg(message& msg){
    try{
        auto task=std::make_shared<hdrtask>((void*)hdrbuf, msg, std::weak_ptr<basic_connection>(shared_from_this()));
        c->add_read(task);
        if(task->wait_for(recv_timeout)){
            if(task->failure){
                c->remove_read(task);
                return status::failure("invalid hdr, msg body size illegal");
            }
            return status::success();
        }else{
            c->remove_read(task->subtask);
            c->remove_read(task);
            return status::failure("timeout");
        }
    }catch(const std::exception& e){
        return status::fault(e.what());
    }catch(...){
        return status::fault("unknown exception");
    }
}



}