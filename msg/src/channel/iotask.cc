#include "iotask.h"

namespace msg{

// return true if current task is successfully done, so that conn can continue.
// return false if read fails or ends(EAGAIN), in which case conn must stop to avoid messing up the correct order of tasks. Note that on_failure callback is triggered, so that task owner may modify something, the conn object will always try to coomplete it as long as the task reserved in the list.
status read_task::try_scatter_input(int fd, int backoff){
    logdebug("scatter input over fd %d", fd);
    while(true){
        int n=readv(fd, iov(), iovcnt());
        logdebug("readv done, n=%d", n);
        if(n>0){ // iov is fully transferred
            on_success(n);
            return status::success();
        }else if(n==0){
            return status::error("peer closed, so should we");
        }else{
            logdebug("read task error: %d, %s\n", errno, strerror(errno));
            switch (errno) {
            case EINTR: // interrupted
                continue; 
            case EAGAIN: 
                return status::failure("EAGAIN, try again");
            default: 
                on_recoverable_failure(10+backoff);
                return status::fault("Something bad but unfatal happens, backoff, come back later");
            }
        }
    }
}

// return false if read fails or ends(EAGAIN)
status write_task::try_gather_output(int fd, int backoff){
    logdebug("gather output over fd %d", fd);
    while(true){
        //writev prints iov content on wsl
        int n=writev(fd, iov(), iovcnt());
        logdebug("writev done, n=%d", n);
        if(n>0){ // iov is fully transferred
            on_success(n);
            return status::success();
        }else if(n==0){
            return status::error("peer closed, so should we");
        }else{
            logdebug("write task error: %d, %s\n", errno, strerror(errno));
            switch (errno) {
            case EINTR: // interrupted
                continue; // try again
            case EAGAIN: // can't proceed
                return status::failure("EAGAIN, try again");
            default: 
                on_recoverable_failure(10+backoff);
                return status::fault("Something bad but unfatal happens, backoff, come back later");
            }
        }
    }
}

}