#include "reactor.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include "event.h"
#include "sig.h"
#include "common/fd.h"
#include "timer.h"

#define max_events 64 

using namespace msg::common;

namespace msg{namespace reactor{

reactor::reactor(){
    epollfd=epoll_create1(EPOLL_CLOEXEC);
    eventfd=efd_open();
    eventfd_event=new event(epollfd, eventfd, [this](int evflag){
        efd_recv(eventfd);
    });
    timerfd_event=new event(epollfd,timerfd_timer.get_timerfd(), [this](int evflag){
        timerfd_timer.on_timerfd_event();
    });
    // signalfd_event=create_signalfd_event(epollfd);
}

reactor::~reactor(){
    closing=true;
    if(background_thread.joinable()){
        background_thread.join(); 
    }
    close(epollfd);
    delete eventfd_event;
    delete timerfd_event;
    // delete signalfd_event;
}

void reactor::wake(){
    efd_send(eventfd);
}

void reactor::submit(const please_cb& cb){
    cbq.enqueue(cb); 
}

void reactor::submit_and_wake(const please_cb& cb){
    cbq.enqueue(cb); 
    wake();
}

void reactor::consume(const struct epoll_event* ev){
    static_cast<event*>(ev->data.ptr)->consume(ev->events & (EPOLLIN|EPOLLOUT|EPOLLERR));
}

bool reactor::in_eventloop(){
    if(background_thread.joinable()){
        return background_thread.get_id()==std::this_thread::get_id();
    }
    return false;
}

// fixme: replace iostream with your logging tools or just exit
void reactor::bad_epollwait(){
    logerr("epoll_wait failed!");
}

void reactor::eventloop(){
    eventfd_event->submit_without_oneshot(EPOLLIN);
    timerfd_event->submit_without_oneshot(EPOLLIN);
    // signalfd_event->submit_without_oneshot(EPOLLIN);
	while(!closing){
		struct epoll_event events[max_events];
		int n = epoll_wait(epollfd,events,max_events,-1);
		if(n<0 && errno==EBADF) bad_epollwait(); 
		for(int j=0; j<n; j++) consume(&events[j]);
        please_cb cb; 
        while (cbq.try_dequeue(cb)) cb();
	}
}

void reactor::start_eventloop(){
    background_thread=std::thread([this]{
        eventloop();
    });
}

}}