#include "connector.h"
#include <sys/epoll.h> //evflags


namespace msg{

status connector::create_event(int newfd){
    e = new event(reactor::instance().epollfd, newfd, [this](int evflag){connect_cb(evflag);});
    if(e->submit(EPOLLOUT)){
        return status::success();
    }else{
        return status::failure("connect event submit failed");
    }
}

status connector::connect(){ 
    return ctask->try_connect(this); 
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
        taskpool::instance().execute([this, evflag]{
            auto s=ctask->handle_async_connect_result();
            if(!s.is_success()) logerr(s.str());
        });
    }
}



}