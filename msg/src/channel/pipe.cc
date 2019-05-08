#include "pipe.h"

#include <sys/epoll.h> //evflags
#include "common/taskpool.h"

namespace msg{

pipe::pipe(int fd){
    e = new event(reactor::instance().epollfd, fd, [this](int evflag){pipe_cb(evflag);});
}

void pipe::add_read(const read_sp& m){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    reads.push_back(m);
    if(reads.size()==1) read(), resubmit_read(); 
}

void pipe::add_write(const write_sp& m){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    writes.push_back(m);
    if(writes.size()==1) write(), resubmit_write(); 
}

void pipe::close(){
    std::lock_guard<std::mutex> lk(mtx);
    if (!closed) {
        doclose();
    }
}

void pipe::pipe_cb(int evflag){
    if(evflag & (EPOLLHUP | EPOLLERR)) {
        close();  // close on orderly shut or error
    }else{ // almost certainly time-consuming  
        taskpool::instance().execute([this, evflag]{
            std::lock_guard<std::mutex> lk(mtx);
            if(evflag&EPOLLIN) read();
            if(evflag&EPOLLOUT) write();
            resubmit_both();
        });
    }
}

void pipe::resubmit_both(){
    if(closed) return;
    int flag = 0; 
    if(!reads.empty()) flag |= EPOLLIN; //1
    if(!writes.empty()) flag |= EPOLLOUT; //4
    if(flag!=0){
        logdebug("now we submit %d", flag); 
        e->submit(flag);
    }
}

void pipe::resubmit_read(){
    if(closed) return;
    if(!reads.empty()) e->submit(EPOLLIN);
}

void pipe::resubmit_write(){
    if(closed) return;
    if(!writes.empty()) e->submit(EPOLLOUT);
}

void pipe::read(){
    if(closed) return ; 
    logdebug("read events over listener");
    while(!reads.empty()) {
        auto& cur=reads.front();
        auto s=cur->try_scatter_input(e->fd);
        if(s.is_success()) reads.pop_front();
        else if(s.is_failure()) return;
        else if(s.is_error()){ 
            doclose();
            return;
        }
    }
}

void pipe::write(){
    if(closed) return;
    logdebug("write events over listener");
    while(!writes.empty()) {
        auto& cur=writes.front();
        auto s=cur->try_gather_output(e->fd);
        if(s.is_success()) writes.pop_front();
        else if(s.is_failure()) return;
        else if(s.is_error()){ 
            doclose();
            return;
        }
    }
}

void pipe::doclose(){
    closed = true;
    for(auto& wr:writes) wr->on_pipe_closed();
    for(auto& rd:reads) rd->on_pipe_closed();
    e->please_destroy_me();
}

}