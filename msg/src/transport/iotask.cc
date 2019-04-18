#include "iotask.h"

namespace msg{namespace transport{

// return true if current task is successfully done, so that conn can continue.
// return false if read fails or ends(EAGAIN), in which case conn must stop to avoid messing up the correct order of tasks. Note that on_failure callback is triggered, so that task owner may modify something, the conn object will always try to coomplete it as long as the task reserved in the list.
bool read_task::try_scatter_input(int fd){
    while(true){
        int n=readv(fd, iov(), iovcnt());
        if(n>0){ // iov is fully transferred
            on_success(n);
            return true;
        }else if(n==0){
            on_failure(peer_closed);
            return false;
        }else{
            switch (errno) {
            case EINTR: // interrupted
                continue; // try again
            case EAGAIN: // can't proceed
                return false; // finish current run
            default: 
                on_failure(bad_but_recoverable); 
                return false; 
            }
        }
    }
}

// return false if read fails or ends(EAGAIN)
bool write_task::try_gather_output(int fd){
    while(true){
        int n=writev(fd, iov(), iovcnt());
        if(n>0){ // iov is fully transferred
            on_success(n);
            return true;
        }else if(n==0){
            on_failure(peer_closed);
            return false;
        }else{
            switch (errno) {
            case EINTR: // interrupted
                continue; // try again
            case EAGAIN: // can't proceed
                return false; // finish current run
            default: 
                on_failure(bad_but_recoverable); 
                return false; 
            }
        }
    }
}

}}