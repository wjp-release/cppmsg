#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "system/reactor.h"
#include "system/timer.h"
#include "common/clock.h"
#include "channel/pipe.h"
#include "channel/connection.h"
#include <functional>
#include "system/addr.h"
#include "system/resolv.h"
#include "channel/basic_connection.h"
#include "session/session.h"
using namespace std;
using namespace msg;

void perfsvr(){
    reactor::instance().start_eventloop();
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,12345,"localhost",1, 0, 0);
    t->wait();
    std::cout<<"now we have parsed addr"<<std::endl;
    int listenfd;
    auto s=bind_listen(t->parsed_address,listenfd);
    std::cout<<"listen "<<s.str()<<", listenfd="<<listenfd<<std::endl;
    int connfd;
    s=sync_accept(listenfd, connfd);
    std::cout<<"accept "<<s.str()<<", connfd="<<connfd<<std::endl;
    auto c=basic_connection::make(connfd);
    for(uint64_t i=0;i<100;i++){
        message what;
        std::cout<<"try to recv msg"<<i<<std::endl;
        c->recvmsg(what);
        std::cout<<"msg"<<i<<" recved: "<<what.str()<<std::endl;
        what.print();
    }
}

                           
int main() {  
    perfsvr();
    while(true){}
    return 0;
}
