#include "event.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/epoll.h>
#include "fd.h"

namespace msg{ 

event::event(int epollfd, int fd, const event_cb& cb):cb(cb),fd(fd){
    set_nonblock(fd);
    ::epoll_event ee;
    ee.events=0;
    ee.data.ptr=this;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ee)!=0){
        logerr("epoll_ctl add failed, errno=%d, err is \"%s\", fd=%d, epollfd=%d", errno, strerror(errno), fd, epollfd);
        throw std::runtime_error("epoll_ctl add failed");
    }
        
}
event::~event(){
    ::epoll_event ee; //unused
    epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_DEL, fd, &ee);
    close(fd);
}

bool event::submit_without_oneshot(int evflag){
    std::lock_guard<std::mutex> lk(mtx);
    if (closing) return false; 
	this->evmask |= evflag; 
    ::epoll_event ee;
    ee.events=evmask|EPOLLERR;
    ee.data.ptr=this;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_MOD, fd, &ee)==0;
}

bool event::submit(int evflag){
    std::lock_guard<std::mutex> lk(mtx);
    if (closing) return false; 
	this->evmask |= evflag; 
    ::epoll_event ee;
    ee.events=evmask|EPOLLONESHOT|EPOLLERR;
    ee.data.ptr=this;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_MOD, fd, &ee)==0;
}

bool event::submit_in(){
    return submit(EPOLLIN);
}

bool event::submit_out(){
    return submit(EPOLLOUT);
}

bool event::submit_both(){
    return submit(EPOLLOUT|EPOLLIN);
}

void event::consume(int evflag){
    std::lock_guard<std::mutex> lk(mtx);
    //logdebug("event captured over fd %d", fd);
    this->evmask &= ~evflag;
    if(cb) cb(evflag);
}

// (mtx won't work) we have to submit delete operation to the eventloop to make sure the deleted event won't be captured by eventloop at the same time.
void event::please_destroy_me(){
    reactor::instance().submit_and_wake([this]{
        delete this; // safely deleted in eventloop
    });
}

void event::set_cb(event_cb cb_){
    std::lock_guard<std::mutex> lk(mtx);
    this->cb=cb_;
}

}