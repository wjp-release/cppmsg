#pragma once

#include "def.h"
#include "reactor/event.h"
#include "transport/iotask.h"
#include "common/taskpool.h"

#include <mutex>
#include <sys/epoll.h> //evflags
#include <list>

namespace msg{namespace transport{

// Class conn is an abstraction of an established 2-way connection
class conn{
public:
    using write_sp=std::shared_ptr<write_task>;
    using read_sp=std::shared_ptr<read_task>;
    conn(int fd){
        e = new reactor::event(reactor::reactor::instance().epollfd, fd, 
            [this](int evflag){conn_cb(evflag);}
        );
    }
    ~conn(){
        if(!closed) close();
    }
    void                add_read(const read_sp& m){
        std::lock_guard<std::mutex> lk(mtx);
        if(closed) return;
        reads.push_back(m);
        if(reads.size()==1) read(), resubmit_read(); 
    }
    void                add_write(const write_sp& m){
        std::lock_guard<std::mutex> lk(mtx);
        if(closed) return;
        writes.push_back(m);
        // list::size takes constant time since c++11
        if(writes.size()==1) write(), resubmit_write(); 
    }
    void                close(){
        std::lock_guard<std::mutex> lk(mtx);
        if (!closed) {
            closed = true;
            //trigger on_failure callbacks of pending reads/writes
            for(auto& wr:writes) wr->on_failure(conn_closed);
            for(auto& rd:reads) rd->on_failure(conn_closed);
            e->please_destroy_me();
        }
    }
protected:
    void                conn_cb(int evflag){
        if(evflag & (EPOLLHUP | EPOLLERR)) {
            close();  // close on orderly shut or error
        }else{ // almost certainly time-consuming  
            common::taskpool::instance().execute([this, evflag]{
                std::lock_guard<std::mutex> lk(mtx);
                if(evflag&EPOLLIN) read();
                if(evflag&EPOLLOUT) write();
                resubmit_both();
            });
        }
    }
    void                resubmit_both(){
        if(closed) return;
        int flag = 0; 
        if(!reads.empty()) flag |= EPOLLIN;
        if(!writes.empty()) flag |= EPOLLOUT;
        if(flag!=0) e->submit(flag);
    }
    void                resubmit_read(){
        if(closed) return;
        if(!reads.empty()) e->submit(EPOLLIN);
    }
    void                resubmit_write(){
        if(closed) return;
        if(!writes.empty()) e->submit(EPOLLOUT);
    }
    void                read(){
        std::cout<<"read event over conn!\n";
        if(closed) return;
        while(!reads.empty()) {
            auto& cur=reads.front();
            if(!cur->try_scatter_input(e->fd)) return; 
            else reads.pop_front();
        }
    }
    void                write(){
        std::cout<<"read event over conn!\n";
        if(closed) return;
        while(!writes.empty()) {
            auto& cur=writes.front();
            if(!cur->try_gather_output(e->fd)) return;
            else writes.pop_front();
        }
    }
private:
    msg::reactor::event*   e; 
    bool                   closed = false; 
    std::list<write_sp>    writes;
    std::list<read_sp>     reads; 
    std::mutex             mtx;
};

}} 