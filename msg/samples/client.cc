#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "system/reactor.h"
#include "system/timer.h"
#include "common/clock.h"
#include "protocol/channel/conn.h"
#include "protocol/channel/connection.h"
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
    message_connection c(connfd);
    message what;
    for(int i=0;i<10;i++){
        std::cout<<"try to send msg"<<i<<std::endl;
        c.sendmsg("Client request <"+std::to_string(i)+">");
        std::cout<<"msg"<<i<<" sent!"<<std::endl;
        std::cout<<"try to recv msg"<<i<<"'s reply"<<std::endl;
        c.recvmsg(what);
        std::cout<<"msg"<<i<<"'s reply recved!"<<std::endl;
        what.print();
    }
}

int main() {
    simple_msgconn_client();
    cin.get();
    return 0;
}

