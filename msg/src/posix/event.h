#pragma once



namespace msg{
namespace posix{



enum event_type : uint8_t{
    SOCK_EV     = 0, //socket
    TIMER_EV    = 1, //timer
    USER_EV     = 2, //user-defined 
    SIG_EV      = 3, //singal
};                      

class event{
public:

private:
    reactor* reactor;
    int             fd;  
    event_cb cb;
    void*           arg;
    int             evmask;
    bool            closing; 
    uint8_t         type;   
    pthread_mutex_t mtx;
};



}
}