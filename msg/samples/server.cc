#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "reactor/reactor.h"
#include "reactor/timer.h"
#include "common/clock.h"
#include "transport/conn.h"
#include "sample.h"
#include <functional>

using namespace std;
using namespace msg::sample;
using namespace msg;

// server
// unix domain socket event
// timeout event : every 1s write something to client

#define readbuf_size 1024

void tcpconn_server(){
    reactor::reactor::instance().start_eventloop();
    reactor::reactor::instance().get_timer().please_push([]{
        cout<<"timeout~"<<endl;
    }, common::future(1000), 3000);
    int connfd=ipc_bind(server_uds_path);
    transport::conn c(connfd);

}

int main() {
    tcpconn_server();
    cin.get();
    return 0;
}

