#include "event.h"
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/epoll.h>

namespace msg{ namespace posix{

bool event::epoll_add(){
    struct epoll_event ee;
    ee.events=0;
    ee.data.ptr=this;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_ADD, fd, &ee)==0;
}

bool event::epoll_del(){
    struct epoll_event ee;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_DEL, fd, &ee)==0;
}

bool event::epoll_mod(){
    struct epoll_event ee;
    ee.events=evmask|EPOLLONESHOT|EPOLLERR;
    ee.data.ptr=e;
    return epoll_ctl(reactor::instance().epollfd, EPOLL_CTL_MOD, fd, &ee)==0;
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

bool event::please_submit(int evflag){
    reactor::instance().run([this, evflag]{
        submit(evflag);
    });
}

}}