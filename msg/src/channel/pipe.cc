#include "pipe.h"

#include <sys/epoll.h> //evflags
#include "common/taskpool.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>

namespace msg{
#ifndef TCP_NODELAY
#define TCP_NODELAY 1
#endif

// disable nagle
static status set_nodelay(int fd, int nodelay){
	int r=setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(int));
    if(r==0) return status::success();
    else return status::failure("setsockopt tcp nodelay failed");
}

static status set_keepalive(int fd, int keepalive){
	int r=setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &keepalive, sizeof(int));
    if(r==0) return status::success();
    else return status::failure("setsockopt tcp nodelay failed");
}

status pipe::enable_nagle(){
    set_nodelay(e->fd, 0);
}

status pipe::disable_nagle(){
    set_nodelay(e->fd, 1); 
}

status pipe::enable_keepalive(){
    set_keepalive(e->fd, 1);
}

status pipe::disable_keepalive(){
    set_keepalive(e->fd, 0);
}

pipe::pipe(int fd){
    e = new event(reactor::instance().epollfd, fd, [this](int evflag){pipe_cb(evflag);});
    // nagle enabled by default
    set_keepalive(fd, 1);
}

uint16_t pipe::get_backoff() const noexcept{
    std::lock_guard<std::mutex> lk(mtx);
    return backoff;
}

void pipe::add_read(const read_sp& m){
    std::unique_lock<std::mutex> lk(mtx);
    doadd_read(lk,m);
}

void pipe::add_write(const write_sp& m){
    std::unique_lock<std::mutex> lk(mtx);
    doadd_write(lk, m);
}

void pipe::doadd_read(std::unique_lock<std::mutex>&lk, const read_sp& m){
    if(closed) return;
    reads.push_back(m);
    if(reads.size()==1) dosubmit_read();
    
    // What should we do right after adding a read task? 
    // Should we submit only(and wait for epoll event) or try to doread aggressively, assuming that data is ready?

    // We choose submit only here, as it has better sendmsg_async performance

    // if(reads.size()==1){
    //     doread(lk);
    //     dosubmit_read();
    // } 
}

void pipe::doadd_write(std::unique_lock<std::mutex>&lk, const write_sp& m){
    if(closed) return;
    writes.push_back(m);
    if(writes.size()==1) dosubmit_write();
}

void pipe::remove_write(const write_sp& m){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    writes.remove(m);
}

void pipe::remove_read(const read_sp& m){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    reads.remove(m);
}

void pipe::clear_writes(){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    writes.clear();
}

void pipe::clear_reads(){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    reads.clear();
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
            std::unique_lock<std::mutex> lk(mtx);
            if(evflag&EPOLLIN) doread(lk);
            if(evflag&EPOLLOUT) dowrite(lk);
            dosubmit_both();
        });
    }
}

void pipe::resubmit_both(){
    std::lock_guard<std::mutex> lk(mtx);
    dosubmit_both();
}

void pipe::resubmit_read(){
    std::lock_guard<std::mutex> lk(mtx);
    dosubmit_read();
}

void pipe::resubmit_write(){
    std::lock_guard<std::mutex> lk(mtx);
    dosubmit_write();
}

void pipe::read(){
    std::unique_lock<std::mutex> lk(mtx);
    doread(lk);
}

void pipe::write(){
    std::unique_lock<std::mutex> lk(mtx);
    dowrite(lk);
}

void pipe::dosubmit_both(){
    if(closed) return;
    int flag = 0; 
    if(!reads.empty()) flag |= EPOLLIN; //1
    if(!writes.empty()) flag |= EPOLLOUT; //4
    if(flag!=0){
        logdebug("now we submit %d", flag); 
        e->submit(flag);
    }
}

void pipe::dosubmit_read(){
    if(closed) return;
    if(!reads.empty()) e->submit(EPOLLIN);
}

void pipe::dosubmit_write(){
    if(closed) return;
    if(!writes.empty()) e->submit(EPOLLOUT);
}

void pipe::doread(std::unique_lock<std::mutex>& lk){
    if(closed) return ; 
    logdebug("read events over listener");
    while(!reads.empty()) {
        auto& cur=reads.front();
        lk.unlock();
        auto s=cur->try_scatter_input(e->fd, backoff, lk);
        lk.lock();
        if(s.is_success()){ // good, pop it
            reads.pop_front();
            backoff=0; // turn off backoff
        }else if(s.is_failure()){ // retry immediately
            backoff=0; // turn off backoff
            return;
        }else if(s.is_error()){ // can't recover, we are doomed
            doclose();
            return;
        }else if(s.is_fault()){ // come back later
            // The first task run into this problem will schedule a timeout, resubmit read later.
            adjust_backoff(); // incremental backoff 
        }
    }
}

void pipe::dowrite(std::unique_lock<std::mutex>& lk){
    if(closed) return;
    logdebug("write events over listener");
    while(!writes.empty()) {
        auto& cur=writes.front();
        lk.unlock();
        auto s=cur->try_gather_output(e->fd, backoff, lk);
        lk.lock();
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

void pipe::adjust_backoff(){
    if(backoff!=0){
        if(backoff<backoff_max){  
            backoff*=2; 
        }
    }else{
        backoff=10;  // starts from 10
    }
}


}