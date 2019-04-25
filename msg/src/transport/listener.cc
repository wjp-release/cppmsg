#include "listener.h"
#include "common/taskpool.h"
#include "common/fd.h"

#include <sys/epoll.h> //evflags
#include <arpa/inet.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

namespace msg{namespace transport{

listener::listener(const addr& a){
    // [1] create socket
    int fd = socket(a.posix_family(), SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(fd<0){
        throw std::runtime_error("create socket failed");
    }
    msg::common::set_sockfd_reuse_addr(fd);
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
    e = new reactor::event(reactor::reactor::instance().epollfd, fd, [this](int evflag){listener_cb(evflag);});
    // We don't submit it now. We should only submit(EPOLLIN) when nonblocking accept is unfinished!
}

void listener::listener_cb(int evflag){
    if(evflag & (EPOLLHUP | EPOLLERR)) {
        close();
    }else{ // almost certainly time-consuming  
        common::taskpool::instance().execute([this, evflag]{
            std::lock_guard<std::mutex> lk(mtx);
            accept(), resubmit_accept();
        });
    }
}
void listener::resubmit_accept(){ 
    if(closed) return;
    if(!accepts.empty()) e->submit(EPOLLIN);
}
void listener::accept(){
    logdebug("accept events over listener");
    if(closed) return;
    for(auto i=accepts.begin(), d=accepts.end();i!=d;){
        auto rc=(*i)->try_accept(e->fd);
        if(rc == should_stop_all){
            return;
        }else if(rc==ignore_this_continue){
            i++;
            continue;
        }else{ // this task is either done or dropped for fatal err, erase it
            i=accepts.erase(i);
        }
    }
}

void listener::close(){
    std::lock_guard<std::mutex> lk(mtx);
    if (!closed) {
        closed = true;
        //trigger on_failure callbacks of pending accepts
        for(auto& a:accepts) a->on_failure(listener_closed);
        e->please_destroy_me();
    }
}

void listener::add_accept(const accept_sp& a){
    std::lock_guard<std::mutex> lk(mtx);
    if(closed) return;
    accepts.push_back(a);
    if(accepts.size()==1) accept(), resubmit_accept(); 
}



}}