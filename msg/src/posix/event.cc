#include "event.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/epoll.h>

namespace msg{ namespace posix{

bool event::epoll_add(){
    ::epoll_event ee;
    ee.events=0;
    ee.data.ptr=this;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_ADD, fd, &ee)==0;
}

bool event::epoll_del(){
    ::epoll_event ee;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_DEL, fd, &ee)==0;
}

bool event::epoll_mod(){
    ::epoll_event ee;
    ee.events=evmask|EPOLLONESHOT|EPOLLERR;
    ee.data.ptr=this;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_MOD, fd, &ee)==0;
}

bool event::epoll_mod_no_oneshot(){
    ::epoll_event ee;
    ee.events=evmask|EPOLLERR;
    ee.data.ptr=this;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_MOD, fd, &ee)==0;
}

bool event::submit_no_oneshot(int evflag){
    if (closing) return false; 
	this->evmask |= evflag; 
    return epoll_mod_no_oneshot();
}

bool event::submit(int evflag){
    if (closing) return false; 
	this->evmask |= evflag; 
    return epoll_mod();
}

func event::consume(int evflag){
    this->evmask &= ~evflag;
    return cb;
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