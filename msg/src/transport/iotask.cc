#include "iotask.h"

namespace msg{namespace transport{

// return false if read fails or ends(EAGAIN)
bool read_task::try_scatter_input(int fd){
    while(true){
        int n=readv(fd, iovs.data(), iovs.size());
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
        int n=writev(fd, iovs.data(), iovs.size());
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