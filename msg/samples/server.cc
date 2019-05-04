#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "system/reactor.h"
#include "system/timer.h"
#include "common/clock.h"
#include "channel/conn.h"
#include "channel/connection.h"
#include "sample.h"
#include <functional>
#include "system/addr.h"
#include "system/resolv.h"
#include "endpoint.h"
#include "channel/basic_connection.h"
using namespace std;
using namespace msg::sample;
using namespace msg;

// server
// unix domain socket event
// timeout event : every 1s write something to client

#define readbuf_size 1024

void add_timer(){
    reactor::instance().get_timer().please_push([]{
        cout<<"timeout~"<<endl;
    }, future(1000), 3000);
}

void simple_msgconn_server(){
    reactor::instance().start_eventloop();
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,12345,"localhost",1, 0, 0);
    t->wait();
    std::cout<<"now we have parsed addr"<<std::endl;
    endpoint ep;
    int listenfd;
    auto s=ep.listen(t->parsed_address,listenfd);
    std::cout<<"listen "<<s.str()<<std::endl;
    int connfd;
    s=ep.accept(listenfd, connfd);
    std::cout<<"accept "<<s.str()<<std::endl;
    basic_connection c(connfd);
    message what;
    for(int i=0;i<100;i++){
        std::cout<<"try to recv msg"<<i<<std::endl;
        c.recvmsg(what);
        std::cout<<"msg"<<i<<" recved!"<<std::endl;
        what.print();
        std::cout<<"try to send msg"<<i<<"'s reply"<<std::endl;
        c.sendmsg("Server response <"+std::to_string(i)+">");
        std::cout<<"msg"<<i<<"'s reply is sent!"<<std::endl;
    }
}

int main() {
    simple_msgconn_server();
    cin.get();
    return 0;
}

