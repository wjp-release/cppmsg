#include "basic_connection.h"
#include "system/reactor.h"
namespace msg{

basic_connection::basic_connection(int fd):connection(fd){
    backoff_routine=[this]{
        c->resubmit_write(); // backoff resubmission
    };
}

status basic_connection::sendmsg_async(const message& msg, const async_cb& cb){
    class send_async_task : public vector_write_task{
    public:
        send_async_task(const message& msg, const connptr& c, const async_cb& cb) : vector_write_task(msg.size(), c), cb(cb){
            msg.append_to_iovs(iovs);

            std::string tmp((const char*)iovs[1].iov_base, iovs[1].iov_len);
            logdebug("create async task msg=[%s], len=%d",tmp.c_str(), tmp.size());
        }
        virtual void on_success(int bytes, std::unique_lock<std::mutex>& lk){
            std::string tmp((const char*)iovs[1].iov_base, iovs[1].iov_len);
            logdebug("send async %d bytes, msg=[%s], msglen=%d", bytes, tmp.c_str(), tmp.size());
            cb(bytes);
        }
        virtual void on_recoverable_failure(int backoff){
            auto c=conn.lock();
            if(!c){ 
                logerr("This task's owning connection has been destroyed, hence this task should have been discarded. Therefore we won't do anything to recover it.");
            }else{
                if(backoff>=backoff_subjectively_down){
                    // async timeout by watching incremental backoff
                    cb(AsyncTimeout); 
                }else{
                    cb(WillRetryLater); 
                    logdebug("async send failed, retry in %d ms", backoff+backoff_base);
                    defer(c->backoff_cb(), backoff);
                }
            }
        }
    private:
        async_cb cb;
    };
    // Try to send a message asynchronously
    try{
        std::weak_ptr<basic_connection> cptr=shared_from_this();
        auto task=std::make_shared<send_async_task>(msg, cptr, cb);
        c->add_write(task);
        return status::success();
    }catch(const std::exception& e){ 
        logerr(e.what());
        return status::fault(e.what());
    }catch(...){
        logerr("unknown exception");
        return status::fault("unknown exception");
    }

}


status basic_connection::sendmsg(const message& msg){
    class sendtask : public vector_write_task, public blockable{
    public:
        sendtask(const message& msg, const connptr& c) : vector_write_task(msg.size(), c){
            msg.append_to_iovs(iovs); 
            std::string tmp((const char*)iovs[1].iov_base, iovs[1].iov_len);
            logdebug("create sync task msg=[%s], len=%d",tmp.c_str(), tmp.size());

        }
        virtual ~sendtask(){}
        virtual void on_success(int bytes, std::unique_lock<std::mutex>& lk){
            std::string tmp((const char*)iovs[1].iov_base, iovs[1].iov_len);
            logdebug("send async %d bytes, msg=[%s], msglen=%d", bytes, tmp.c_str(), tmp.size());

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
    // Try to send a message synchronously
    try{
        std::weak_ptr<basic_connection> cptr=shared_from_this();
        auto task=std::make_shared<sendtask>(msg, cptr);
        c->add_write(task);
        if(task->wait_for(send_timeout)){
            logdebug("good, the task is done");
            return status::success();
        }else{
            logerr("timeout, remove the task");
            c->remove_write(task);
            return status::failure("timeout");
        }            
    }catch(const std::exception& e){ 
        logerr(e.what());
        return status::fault(e.what());
    }catch(...){
        logerr("unknown exception");
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
        if(!msg.empty()){msg.clear();}
        std::weak_ptr<basic_connection> cptr=shared_from_this();
        auto task=std::make_shared<hdrtask>((void*)hdrbuf, msg, cptr);
        c->add_read(task);
        if(task->wait_for(recv_timeout)){
            if(task->failure){
                c->remove_read(task);
                logerr("invalid hdr");
                return status::failure("invalid hdr, msg body size illegal");
            }
            logdebug("good, the task is done");
            return status::success();
        }else{
            c->remove_read(task->subtask);
            c->remove_read(task);
            logerr("timeout, remove the task and its subtask");
            return status::failure("timeout");
        }
    }catch(const std::exception& e){
        logerr(e.what());
        return status::fault(e.what());
    }catch(...){
        logerr("unknown exception");
        return status::fault("unknown exception");
    }
}



}