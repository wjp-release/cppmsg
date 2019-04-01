#pragma once

#include "def.h"
#include "reactor.h"
#include <unistd.h>

namespace msg{
namespace posix{

enum event_type : uint8_t{
    SOCK_EV     = 0, //socket fd
    TIMER_EV    = 1, //timerfd
    USER_EV     = 2, //eventfd 
    SIG_EV      = 3, //singalfd
};                 

/*
    How can we prevent race conditions over event data member access and modifications?
    
    Plan A: Use mutex (except for destruction of events, which must be synchronized with eventloop); 
    
    Plan B: Wrap event modification operation as a functional object that will later be executed sequentially in reactor's event loop.

    Plan B saves size of mutex space for each event. Considering user code probably won't modify evflag or cb of existing events too often, compromising certain degree of parallelism to me is acceptable. 

    Methods with prefix 'please_' must be called by user threads. They will return immediately and actually been run in the eventloop.
*/

class event{
public:
    event(int fd, uint8_t type, const func& cb):cb(cb),fd(fd),type(type){
        set_nonblock(fd);
        epoll_add();
    }
    ~event(){
        close(fd);
    }
    void            please_set_cb(func);// must be called by user thread; let eventloop run set_cb 
    void            please_submit(int evflag); // must be called by user thread; let eventloop run submit 
    func            consume(int evflag); // must be called by eventloop
    bool            submit_no_oneshot(int evflag);  // must be called by eventloop; timerfd, eventfd & signalfd
private:
    bool            submit(int evflag); 
    bool            epoll_mod_no_oneshot();
    bool            epoll_add();
    bool            epoll_del();
    bool            epoll_mod();
    func            cb;
    int             fd;  
    int             evmask  = 0; //what's under watch
    bool            closing = false; 
    uint8_t         type;   
};



}
}