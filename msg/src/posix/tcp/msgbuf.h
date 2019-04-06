#pragma once

#include "def.h"
#include "taskpool.h"
#include <sys/uio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <vector>

/*
    struct iovec {
        void  *iov_base; // Starting address 
        size_t iov_len; // Number of bytes to transfer 
    };
*/

namespace msg{namespace posix{namespace tcp{

enum msg_transfer_failure : uint8_t{
    peer_closed = 1,
    other = 2, 
    conn_closed =3,
};

class msgbuf{
public:
    msgbuf(){}
    msgbuf(const transferred_cb& tcb, const failure_cb& fcb, int nr_iov):tcb(tcb),fcb(fcb){
        
    }
    ~msgbuf(){

    }
    void        on_transferred(){
        if(tcb){
            taskpool::instance().execute(tcb);
        }
    }
    void        on_failure(int what){
        if(fcb){
            taskpool::instance().execute([this,what]{
                fcb(what);
            });
        }
    }
    // return false if read fails or ends(EAGAIN)
    bool         try_gather_read(int fd){
        while(true){
            int n=readv(fd, iovs.data(), iovs.size());
            if(n>0){
                on_transferred();
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
                default: // todo: support more err codes
                    on_failure(other); 
                    return false; // 
                }
            }
        }
    }

    // return false if read fails or ends(EAGAIN)
    bool         try_scatter_write(int fd){
        while(true){
            int n=writev(fd, iovs.data(), iovs.size());
            if(n>0){
                on_transferred();
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
                default: // todo: support more err codes
                    on_failure(other); 
                    return false; // 
                }
            }
        }
    }
private:
    std::vector<struct iovec> iovs;
    transferred_cb tcb;
    failure_cb     fcb;
};



}}}