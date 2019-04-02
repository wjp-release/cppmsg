#include "event.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/epoll.h>

namespace msg{ namespace posix{

event::event(int epollfd, int fd, uint8_t type, const func& cb):cb(cb),fd(fd),type(type){
    set_nonblock(fd);
    ::epoll_event ee;
    ee.events=0;
    ee.data.ptr=this;
    if(epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ee)!=0)
        throw std::runtime_error("can't create epollfd");
}
event::~event(){
    ::epoll_event ee; //unused
    epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_DEL, fd, &ee);
    close(fd);
}

bool event::submit_without_oneshot(int evflag){
    if (closing) return false; 
	this->evmask |= evflag; 
    ::epoll_event ee;
    ee.events=evmask|EPOLLERR;
    ee.data.ptr=this;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_MOD, fd, &ee)==0;
}

bool event::submit(int evflag){
    if (closing) return false; 
	this->evmask |= evflag; 
    ::epoll_event ee;
    ee.events=evmask|EPOLLONESHOT|EPOLLERR;
    ee.data.ptr=this;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_MOD, fd, &ee)==0;
}

void event::consume(int evflag){
    this->evmask &= ~evflag;
    if(cb) cb();
}

void event::please_destroy_me(){
    reactor::instance().run([this]{
        delete this; // safely deleted in eventloop
    });
}

void event::please_set_cb(func cb_){
    reactor::instance().run([this, cb_]{
        this->cb=cb_;
    });
}

void event::please_submit(int evflag){
    reactor::instance().run([this, evflag]{
        submit(evflag);
    });
}

}}