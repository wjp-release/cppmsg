#include "msgbuf.h"
#include <sys/types.h>
#include <sys/socket.h>

namespace msg{namespace posix{

namespace detail{

/*
    struct iovec {
        void  *iov_base; // Starting address 
        size_t iov_len; // Number of bytes to transfer 
    };

    Buffers are processed in array order.  This means that readv() completely fills iov[0] before proceeding to iov[1], and so on.  (If there is insufficient data, then not all buffers pointed to by iov may be filled.)  Similarly, writev() writes out the entire contents of iov[0] before proceeding to iov[1], and so on.

    The data transfers performed by readv() and writev() are atomic: the data written by writev() is written as a single block that is not intermingled with output from writes in other processes ; analogously, readv() is guaranteed to read a contiguous block of data from the file.
*/

class msgbuf_impl{
public:
    msgbuf_impl(){}
    msgbuf_impl(const transferred_cb& tcb, const failure_cb& fcb, int nr_iov):tcb(tcb),fcb(fcb),iovs(nr_iov)
    {
        
    }
    ~msgbuf_impl(){}
    // msgbuf does not alloc, nor dealloc memory for iovs
    void        add_iov(void* addr, size_t len){
        iovs.push_back({addr, len});
    }
    void        on_transferred(){
        if(tcb){
            taskpool::instance().execute([this]{
                tcb(iovs);
            });
        }
    }
    void        on_failure(int what){
        if(fcb){
            taskpool::instance().execute([this,what]{
                fcb(what, iovs);
            });
        }
    }
    // return false if read fails or ends(EAGAIN)
    bool         try_scatter_input(int fd){
        while(true){
            int n=readv(fd, iovs.data(), iovs.size());
            if(n>0){ // iov is fully transferred
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
    bool         try_gather_output(int fd){
        while(true){
            int n=writev(fd, iovs.data(), iovs.size());
            if(n>0){ // iov is fully transferred
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

} // detail

msgbuf::msgbuf(const transferred_cb& tcb, const failure_cb& fcb, int nr_iov) : mptr(std::make_shared<detail::msgbuf_impl>(tcb, fcb, nr_iov)){}
msgbuf& msgbuf::operator=(const msgbuf& other) noexcept{
    mptr=other.mptr;
    return *this;
}
void msgbuf::on_transferred(){
    if(mptr) mptr->on_transferred();
}
void msgbuf::on_failure(int what){
    if(mptr) mptr->on_failure(what);
}
bool msgbuf::try_scatter_input(int fd){
    if(mptr) mptr->try_scatter_input(fd);
}
bool msgbuf::try_gather_output(int fd){
    if(mptr) mptr->try_gather_output(fd);
}
void msgbuf::add_iov(void* addr, size_t len){
    if(mptr) mptr->add_iov(addr, len);
}

}}