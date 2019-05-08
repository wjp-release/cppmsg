#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "system/reactor.h"
#include "system/timer.h"
#include "common/clock.h"
#include "channel/pipe.h"
#include "channel/connection.h"
#include "sample.h"
#include <functional>
#include "system/addr.h"
#include "system/resolv.h"
#include "session.h"
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
    session ep;
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

void simple_conn_server(){
    // init
    reactor::instance().start_eventloop();
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,12345,"localhost",1, 0, 0);
    t->wait();
    std::cout<<"now we have parsed addr"<<std::endl;
    session ep;
    int listenfd;
    auto s=ep.listen(t->parsed_address,listenfd);
    std::cout<<"listen "<<s.str()<<", listenfd="<<listenfd<<std::endl;
    int connfd;
    s=ep.accept(listenfd, connfd);
    std::cout<<"accept "<<s.str()<<", connfd="<<connfd<<std::endl;
    // conn
    struct tmp_task : public oneiov_read_task{
        tmp_task() : oneiov_read_task(tmp, 1024)
        {}
        char tmp[1024];
        void on_success(int bytes){
            logdebug("%d bytes read: %s", bytes, std::string(tmp, bytes).c_str());
        }
        void on_recoverable_failure(){
            logerr("tmp_task failed");
            exit(-1);
        }
    };
    msg::pipe c(connfd);
    std::cout<<"pipe created"<<std::endl;
    c.add_read(std::make_shared<tmp_task>());
    c.add_read(std::make_shared<tmp_task>());
    c.add_read(std::make_shared<tmp_task>());
    c.add_read(std::make_shared<tmp_task>());
    std::cout<<"4 read tasks added"<<std::endl;
    while(true){}
}

int main() {
    simple_conn_server();
    while(true){}
    return 0;
}

