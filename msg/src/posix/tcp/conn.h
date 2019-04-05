#pragma once

#include "def.h"
#include "posix/event.h"
#include "msg.h"
#include <mutex>
#include "taskpool.h"
#include <sys/epoll.h> //evflags

namespace msg{namespace posix{namespace tcp{

class conn{
public:
    conn(int fd){
        e = new event(reactor::instance().epollfd, fd, 
            [this](int evflag){conn_cb(evflag);}
        );
    }
    ~conn(){
        if(!closed) close();
    }
    void                read(){

    }
    void                write(){

    }
    void                close(){
        std::lock_guard<std::mutex> lk(mtx);
        if (!closed) {
            closed = true;
            //trigger on_failure callbacks of pending reads/writes
            for(auto& wr:pending_writes) wr.on_failure();
            for(auto& rd:pending_reads) rd.on_failure();
            e->please_destroy_me();
        }

    }
    void                conn_cb(int evflag){
        // EPOLLHUP and EPOLLERR will always be reported 
        // EPOLLHUP: peer orderly shutdown, both read and write
        // EPOLLERR: error happens
        if(evflag & (EPOLLHUP | EPOLLERR)) {
            close();  
        }else{ // almost certainly time-consuming  
            taskpool::instance().execute([this, evflag]{
                std::lock_guard<std::mutex> lk(mtx);
                if(evflag&EPOLLIN) read();
                if(evflag&EPOLLOUT) write();
                int newflag = 0; // clear evflag
                if(!pending_reads.empty()) newflag |= EPOLLOUT;
                if(!pending_writes.empty()) newflag |= EPOLLIN;
                if(!closed && newflag!=0) e->submit(newflag);
            });
        }
    }
private:
    event*              e; // tcp connection event  
    bool                closed = false; 
    std::vector<msgbuf> pending_writes;
    std::vector<msgbuf> pending_reads;
    std::mutex          mtx;
};

}}} 