#include "reactor.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include "event.h"
#include "fd.h"
#include "timer.h"

#define max_events 64 

namespace msg{

reactor::reactor(){
    epollfd=epoll_create1(EPOLL_CLOEXEC);
    if(epollfd==-1) logerr("epoll_create1 failed! errno=%d, err is: %s", errno,  strerror(errno)), exit(-1);
    eventfd=efd_open();
    if(eventfd==-1) logerr("efd_open failed!"), exit(-1);
    try{
        eventfd_event=new event(epollfd, eventfd, [this](int evflag){
            efd_recv(eventfd);
        });
        timerfd_event=new event(epollfd,timerfd_timer.get_timerfd(), [this](int evflag){
            timerfd_timer.on_timerfd_event();
        });
    }catch(...){
        logerr("create events failed!"), exit(-1);
    }
}

reactor::~reactor(){
    stop();
}

void reactor::stop(){
    closing=true;
    if(background_thread.joinable()){
        background_thread.join(); 
    }
    close(epollfd);
    delete eventfd_event;
    delete timerfd_event;
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

void reactor::eventloop(){
    eventfd_event->submit_without_oneshot(EPOLLIN);
    timerfd_event->submit_without_oneshot(EPOLLIN);
    // signalfd_event->submit_without_oneshot(EPOLLIN);
	while(!closing){
		struct epoll_event events[max_events];
		int n = epoll_wait(epollfd,events,max_events,-1);
        if(n<0) logdebug("epoll_wait error: %d, %s\n", errno, strerror(errno));
    	if(n<0 && errno==EBADF) exit(-1); 
		for(int j=0; j<n; j++) consume(&events[j]);
        please_cb cb; 
        while (cbq.try_dequeue(cb)) cb();
	}
}

void reactor::start_eventloop(){
    background_thread=std::thread([this]{
        try{
            eventloop();
        }catch(std::exception& e){
            logerr("An exception is caught during eventloop:", e.what());
            logdebug("quit now!");
            exit(-1);
        }
    });
}

}