#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "posix/reactor.h"
#include "posix/timer.h"
#include "posix/clock.h"
#include "posix/conn.h"
#include "sample.h"
#include <functional>

using namespace std;
using namespace msg::sample;
using namespace msg::posix;

// server
// unix domain socket event
// timeout event : every 1s write something to client

#define readbuf_size 1024

void tcpconn_server(){
    reactor::instance().start_eventloop();
    reactor::instance().get_timer().please_push([]{
        cout<<"timeout~"<<endl;
    }, future(1000), 3000);
    int connfd=ipc_bind(server_uds_path);
    conn c(connfd);

}

int main() {
    tcpconn_server();
    cin.get();
    return 0;
}

