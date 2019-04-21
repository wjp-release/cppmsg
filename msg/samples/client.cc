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

void simple_msgconn_client(){
    reactor::reactor::instance().start_eventloop();
    int connfd=ipc_connect(server_uds_path);
    protocol::connection c(connfd);
    protocol::message what;
    for(int i=0;i<10;i++){
        c.sendmsg("Client request <"+std::to_string(i)+">");
        c.recvmsg(&what);
        message.print();
    }
}

int main() {
    simple_msgconn_client();
    cin.get();
    return 0;
}

