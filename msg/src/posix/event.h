#pragma once

#include "def.h"
#include "reactor.h"
#include <mutex>

namespace msg{
namespace posix{

enum event_type : uint8_t{
    SOCK_EV     = 0, //socket fd
    TIMER_EV    = 1, //timerfd
    USER_EV     = 2, //eventfd 
    SIG_EV      = 3, //singalfd
};                      

class event{
public:
    event(int fd, uint8_t type, ):fd(fd),type(type){
        set_nonblock(fd);
    }
    ~event(){
        close(fd);
    }
private:
    reactor*        reactor;
    int             fd;  
    int             evmask      = 0;
    bool            closing     = false; 
    uint8_t         type;   
    std::mutex      mtx;
    std::function<void(void)> cb;
};



}
}