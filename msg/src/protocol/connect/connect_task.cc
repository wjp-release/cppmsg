#include "connect_task.h"
#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/eventfd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <sys/types.h>
#include "connector.h"

namespace msg{

status connect_task::try_connect(connector* c)noexcept{
    int fd = socket(a.posix_family(), SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(fd<0) return status::failure("create socket failed");
    this->newfd=fd; 
    addr_posix sp;
    int len=a.to_posix(&sp);
    if(connect(fd, sp.sa(), len)!=0){
        if (errno != EINPROGRESS){
            logerr("connect failed");
            return status::failure("connect failed");
        }
        return status::success();
    }else{ // linux async connect never immediately suceeds though
        return c->create_event(newfd);
    }
}

status connect_task::handle_async_connect_result()noexcept{
    int result=get_sockfd_err(this->newfd);
    if(result<0){  
        if (result == EINPROGRESS) return status::success(); 
        else return on_failure(); 
    }else{
        return on_asnyc_success();
    }
}



    
}