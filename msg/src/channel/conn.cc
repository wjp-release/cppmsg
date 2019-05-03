#include "conn.h"

#include <sys/epoll.h> //evflags
#include "common/taskpool.h"

namespace msg{

conn::conn(int fd){
    e = new event(reactor::instance().epollfd, fd, [this](int evflag){conn_cb(evflag);});
}

void conn::add_read(const read_sp& m){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    reads.push_back(m);
    if(reads.size()==1) read(), resubmit_read(); 
}

void conn::add_write(const write_sp& m){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    writes.push_back(m);
    if(writes.size()==1) write(), resubmit_write(); 
}

void conn::close(){
    std::lock_guard<std::mutex> lk(mtx);
    if (!closed) {
        closed = true;
        //trigger on_failure callbacks of pending reads/writes
        for(auto& wr:writes) wr->on_conn_closed();
        for(auto& rd:reads) rd->on_conn_closed();
        e->please_destroy_me();
    }
}

void conn::conn_cb(int evflag){
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

void conn::resubmit_both(){
    if(closed) return;
    int flag = 0; 
    if(!reads.empty()) flag |= EPOLLIN;
    if(!writes.empty()) flag |= EPOLLOUT;
    if(flag!=0) e->submit(flag);
}

void conn::resubmit_read(){
    if(closed) return;
    if(!reads.empty()) e->submit(EPOLLIN);
}

void conn::resubmit_write(){
    if(closed) return;
    if(!writes.empty()) e->submit(EPOLLOUT);
}

void conn::read(){
    logdebug("read events over listener");
    if(closed) return;
    while(!reads.empty()) {
        auto& cur=reads.front();
        if(!cur->try_scatter_input(e->fd)) return; 
        else reads.pop_front();
    }
}

void conn::write(){
    logdebug("write events over listener");
    if(closed) return;
    while(!writes.empty()) {
        auto& cur=writes.front();
        if(!cur->try_gather_output(e->fd)) return;
        else writes.pop_front();
    }
}


}