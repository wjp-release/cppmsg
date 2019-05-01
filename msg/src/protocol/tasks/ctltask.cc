#include "ctltask.h"
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

namespace msg{

static bool is_recoverable(){
    return (errno==ENETDOWN||errno==ENETDOWN||errno==ENOPROTOOPT||errno==EHOSTDOWN||errno==ENONET||errno==EHOSTUNREACH||errno==EOPNOTSUPP||errno==ENETUNREACH||errno==EAGAIN||errno==EWOULDBLOCK);
}

static bool is_fatal(){
    return errno==EBADF || errno==EFAULT || errno==EINVAL;
}

static bool is_recoverable_but_conn_bad(){
    return errno==ECONNABORTED || errno==ECONNRESET;
}

accept_result accept_task::try_accept(int fd){
    this->newfd = accept4(fd, NULL, NULL, SOCK_CLOEXEC);
    if (newfd < 0) {
        if(is_recoverable()){
            return should_stop_all; 
        }else if(is_fatal()){
            on_failure(fatal); 
            return fatal_err_just_drop_it; 
        }else if(is_recoverable_but_conn_bad()){
            on_failure(bad_but_recoverable); // We should report the error, stop current run.
            return should_stop_all; 
        }else{
            on_failure(trivial); // Errors that does not affect tasks
            return ignore_this_continue; // We should skip this one and continue processing other accepts.
        }
    }
    on_success(0);
    return damn_we_are_good;
}

bool connect_task::try_connect(){
    int fd = socket(a.posix_family(), SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(fd<0){
        throw std::runtime_error("create socket failed");
    }
    this->newfd=fd; 
    addr_posix sp;
    int len=a.to_posix(&sp);
    if(connect(fd, sp.sa(), len)!=0){
        if (errno != EINPROGRESS) logerr("connect failed");
        return false;
    }else{ // linux async connect never immediately suceeds though
        on_success(MSG_CONNECT_SUCCESS_IMMEDIATE); 
        return true;
    }
}

void connect_task::handle_async_connect_result(){
    int result=get_sockfd_err(this->newfd);
    if(result<0){  
        if (result == EINPROGRESS) return; 
        else on_failure(MSG_CONNECT_FATAL_FAILURE); // on_failure should release this connector object
    }else{
        on_success(MSG_CONNECT_SUCCESS); // on_success should release this connector object
    }
}



    
}