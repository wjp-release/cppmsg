#include "connector.h"
#include <sys/epoll.h> //evflags


namespace msg{
    
bool connector::connect(){ 
    bool immediate_connected=ctask->try_connect(); 
    if(!immediate_connected){
        e = new reactor::event(reactor::reactor::instance().epollfd, ctask->newfd, [this](int evflag){connect_cb(evflag);});
        e->submit(EPOLLOUT); // result will be available when fd becomes writable 
    }
    return immediate_connected;
}

void connector::close(){ // cancel async connect event
    std::lock_guard<std::mutex> lk(mtx);
    if (!closed) {
        closed = true;
        e->please_destroy_me();
    }
}

void connector::connect_cb(int evflag){
    if(evflag & (EPOLLHUP | EPOLLERR)) {
        close();
    }else{ // almost certainly time-consuming  
        common::taskpool::instance().execute([this, evflag]{
            ctask->handle_async_connect_result();
        });
    }
}



}