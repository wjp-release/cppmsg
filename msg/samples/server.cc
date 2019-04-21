#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "reactor/reactor.h"
#include "reactor/timer.h"
#include "common/clock.h"
#include "transport/conn.h"
#include "protocol/connection.h"
#include "sample.h"
#include <functional>

using namespace std;
using namespace msg::sample;
using namespace msg;

// server
// unix domain socket event
// timeout event : every 1s write something to client

#define readbuf_size 1024

void simple_msgconn_server(){
    reactor::reactor::instance().start_eventloop();
    reactor::reactor::instance().get_timer().please_push([]{
        cout<<"timeout~"<<endl;
    }, common::future(1000), 3000);
    int connfd=ipc_bind(server_uds_path);
    protocol::connection c(connfd);
    protocol::message what;
    for(int i=0;i<100;i++){
        c.recvmsg(&what);
        message.print();
        c.sendmsg("Server response <"+std::to_string(i)+">");
    }
}

int main() {
    simple_msgconn_server();
    cin.get();
    return 0;
}

