#include "listener.h"
#include "common/taskpool.h"
#include "system/fd.h"
#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/eventfd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <stdlib.h>
#include <string.h>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <sys/epoll.h> //evflags
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace msg{

static bool is_recoverable(){
    return (errno==ENETDOWN||errno==ENETDOWN||errno==ENOPROTOOPT||errno==EHOSTDOWN||errno==ENONET||errno==EHOSTUNREACH||errno==EOPNOTSUPP||errno==ENETUNREACH||errno==EAGAIN||errno==EWOULDBLOCK);
}

listener::listener(const addr& a){
    // [1] create socket
    int fd = socket(a.posix_family(), SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(fd<0){
        throw std::runtime_error("create socket failed");
    }
    set_sockfd_reuse_addr(fd);
    // [2] bind socket
    addr_posix sp;
    int len=a.to_posix(&sp);
    if(bind(fd, sp.sa(), len)<0){
        throw std::runtime_error("bind socket failed");
    }
    // [3] listen (128 is the largest backlog)
    if (listen(fd, 128) != 0) {
        throw std::runtime_error("listen failed");
    }
    // [4] good, let's create a listen event
    e = new event(reactor::instance().epollfd, fd, [this](int evflag){listener_cb(evflag);});
    // We don't submit it now. We should only submit(EPOLLIN) when nonblocking accept is unfinished!
}

void listener::listener_cb(int evflag){
    if(evflag & (EPOLLHUP | EPOLLERR)) {
        close();
    }else{ // almost certainly time-consuming  
        taskpool::instance().execute([this, evflag]{
            this->start();
        });
    }
}
void listener::resubmit_accept(){ 
    if(closed) return;
    e->submit(EPOLLIN);
}
void listener::accept(){
    logdebug("accept events over listener");
    if(closed) return;
    int newfd = accept4(e->fd, NULL, NULL, SOCK_CLOEXEC);
    if (newfd < 0) {
        if(is_recoverable()){
            logerr("accept failed, try it next loop"); 
        }else if(errno==EBADF || errno==EFAULT || errno==EINVAL){
            // 
            closed=true;
            e->please_destroy_me();
            logerr("cannot accept, unrecoverable, close");
        }else if(errno==ECONNABORTED || errno==ECONNRESET){

            logerr("accept failed due to network problem, back off 300ms and retry"); 
        }else{

            logerr("accept failed due to other reason, probably recoverable, back off 100ms and retry");
        }
        return;
    }
    // now we get newfd, create a conn

    
}

void listener::close(){
    std::lock_guard<std::mutex> lk(mtx);
    if (!closed) {
        closed = true;
        e->please_destroy_me();
    }
}

void listener::start(){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    accept(), resubmit_accept(); 
}



}