#include "reactor.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>
#include <iostream>

#include "event.h"
#include "sig.h"
#include "fd.h"
#include "timer.h"

#define max_events 64 

namespace msg{namespace posix{

reactor::reactor(){
    epollfd=epoll_create1(EPOLL_CLOEXEC);
    eventfd=efd_open();
    eventfd_event=new event(eventfd, USER_EV, [this]{
        efd_recv(eventfd);
    });
    timerfd_event=new event(timerfd_timer.get_timerfd(), TIMER_EV, [this]{
        timerfd_timer.on_timerfd_event();
    });
    signalfd_event=create_signalfd_event();
}

reactor::~reactor(){
    closing=true;
    if(background_thread.joinable()){
        background_thread.join(); 
    }
    close(epollfd);
    delete eventfd_event;
    delete timerfd_event;
    delete signalfd_event;
}

void reactor::wake(){
    efd_send(eventfd);
}

void reactor::run(const func& cb){
    enqueue(cb);
    wake();
}

void reactor::run_later(const func& cb){
    enqueue(cb);
}

void reactor::enqueue(const func& cb){
    std::lock_guard<std::mutex> lk(mtx);
    cbq.emplace_back(cb);
}

void reactor::consume(const struct epoll_event* ev){
    event* e = static_cast<event*>(ev->data.ptr);
    func cb=e->consume(ev->events & (EPOLLIN|EPOLLOUT|EPOLLERR));
    if(cb) cb();
}

bool reactor::in_eventloop(){
    if(background_thread.joinable()){
        return background_thread.get_id()==std::this_thread::get_id();
    }
    return false;
}

void reactor::eventloop(){
    eventfd_event->submit_no_oneshot(EPOLLIN);
    timerfd_event->submit_no_oneshot(EPOLLIN);
    signalfd_event->submit_no_oneshot(EPOLLIN);
	while(!closing){
		struct epoll_event events[max_events];
		int n = epoll_wait(epollfd,events,max_events,-1);
		if(n<0 && errno==EBADF) std::cerr<<"epoll_wait failed!\n"; 
		for(int j=0; j<n; j++){
			consume(&events[j]);
		}
        std::vector<func> tmp;
        {
            std::lock_guard<std::mutex> lk(mtx);
            cbq.swap(tmp);
        }
        for(auto& cb: tmp){
            cb();
        }
	}
}

void reactor::start_eventloop(){
    background_thread=std::thread([this]{
        eventloop();
    });
}

}}