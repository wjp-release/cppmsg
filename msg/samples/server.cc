#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "posix/reactor.h"
#include "posix/timer.h"
#include "posix/clock.h"
#include "posix/tcp_impl/conn.h"
#include "posix/msgbuf.h"
#include "sample.h"
#include <functional>

using namespace std;
using namespace msg::sample;
using namespace msg::posix;
using namespace msg::posix::tcpimpl;

// server
// unix domain socket event
// timeout event : every 1s write something to client

#define readbuf_size 1024

struct readbuf{
    char    buf[readbuf_size];
    int     bytes=0;
};

static readbuf buf1;

void tcpconn_server(){
    reactor::instance().start_eventloop();
    reactor::instance().get_timer().please_push([]{
        cout<<"timeout~"<<endl;
    }, future(1000), 3000);
    int connfd=ipc_bind(server_uds_path);
    conn c(connfd);
    readbuf* bufptr=&buf1;
    // create msgbuf with callbacks
    auto tcb=[bufptr](int n, const std::vector<iovec>& iovs){
        bufptr->bytes=n;
        cout<<"now we have read "<<string(bufptr->buf,n)<<endl;
    };
    msgbuf read1(tcb,
    [bufptr](int err, const std::vector<iovec>& iovs){
        cout<<"now we detected an error, err="<<err<<endl;
    },1);
    // alloc iov for msgbuf
    read1.add_iov(buf1.buf, readbuf_size);
    // add read msgbuf to conn, read if reads list empty, resubmit if unfinished
    c.add_read(read1);   

}

int main() {
    tcpconn_server();
    cin.get();
    return 0;
}

