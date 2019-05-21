#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "system/reactor.h"
#include "system/timer.h"
#include "common/clock.h"
#include "channel/pipe.h"
#include "channel/connection.h"
#include "channel/basic_connection.h"
#include "session/session.h"
#include "sample.h"
#include <functional>
#include "system/resolv.h"

using namespace std;
using namespace msg::sample;
using namespace msg;

// server
// unix domain socket event
// timeout event : every 1s write something to client

#define readbuf_size 1024

void simple_msgconn_client(){
    reactor::reactor::instance().start_eventloop();
    int connfd;
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,12345,"localhost",HintActive,HintTCP);
    t->wait();
    std::cout<<"now we have parsed addr"<<std::endl;
    auto s=sync_connect(t->parsed_address,connfd);
    if(!s.is_success()) logerr(s.str().c_str());
    auto c=basic_connection::make(connfd);
    message what;
    for(int i=0;i<10;i++){
        std::cout<<"try to send msg"<<i<<std::endl;
        c->sendmsg("Client request <"+std::to_string(i)+">");
        std::cout<<"msg"<<i<<" sent!"<<std::endl;
        std::cout<<"try to recv msg"<<i<<"'s reply"<<std::endl;
        c->recvmsg(what);
        std::cout<<"msg"<<i<<"'s reply recved!"<<std::endl;
        what.print();
    }
}


void simple_conn_client(){
    reactor::reactor::instance().start_eventloop();
    int connfd=-1;
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,12345,"localhost",HintActive,HintTCP);
    t->wait();
    std::cout<<"now we have parsed addr"<<std::endl;
    auto s=sync_connect(t->parsed_address, connfd);
    if(!s.is_success()) logerr(s.str().c_str());
    std::cout<<"connected, now we try to create pipe"<<std::endl;
    struct tmp_write : public vector_write_task{
        tmp_write(const std::string& what) : vector_write_task( what.size(), std::weak_ptr<basic_connection>()), tmp(what)
        {
            iovs[1].iov_base=(void*)tmp.data();
            iovs[1].iov_len=tmp.size();
        }
        std::string tmp;
        void on_success(int bytes,std::unique_lock<std::mutex>& lk){
            logdebug("%d bytes written: %s", bytes, tmp.c_str());
        }
        void on_recoverable_failure(int backoff){
            logerr("tmp_write failed");
            exit(-1);
        }
    };
    msg::pipe c(connfd);
    std::cout<<"pipe created"<<std::endl;
    c.add_write(std::make_shared<tmp_write>("Hello!"));
    c.add_write(std::make_shared<tmp_write>("Shield against darkness!"));
    std::cout<<"2 write tasks added"<<std::endl;

    while(true){}
}

void basic_once(){
    reactor::reactor::instance().start_eventloop();
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,12345,"localhost",HintActive,HintTCP);
    t->wait();
    std::cout<<"now we have parsed addr"<<std::endl;
    int connfd;
    auto s=sync_connect(t->parsed_address,connfd);
    if(!s.is_success()) logerr(s.str().c_str());
    auto c=basic_connection::make(connfd);
    std::cout<<"try to send msg"<<std::endl;
    c->sendmsg("Damn, we are good!");
    std::cout<<"msg has been sent"<<std::endl;
    while(true){}
}

void basic_100(){
    reactor::reactor::instance().start_eventloop();
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,12345,"localhost",HintActive,HintTCP);
    t->wait();
    std::cout<<"now we have parsed addr"<<std::endl;
    int connfd;
    auto s=sync_connect(t->parsed_address,connfd);
    if(!s.is_success()) logerr(s.str().c_str());
    auto c=basic_connection::make(connfd);
    std::cout<<"try to send msg"<<std::endl;
    for(int i=0;i<100;i++){
        c->sendmsg("~basic~"+std::to_string(i));
        std::cout<<"msg"<<i<<" has been sent"<<std::endl;
    }
    while(true){}
}

static std::string star(int n){
    std::string x="";
    for(int i=0;i<n;i+=10)x+="*";
    return x;
}

void async_100(){
    reactor::reactor::instance().start_eventloop();
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,12345,"localhost",HintActive,HintTCP);
    t->wait();
    std::cout<<"now we have parsed addr"<<std::endl;
    int connfd;
    auto s=sync_connect(t->parsed_address,connfd);
    if(!s.is_success()) logerr(s.str().c_str());
    auto c=basic_connection::make(connfd);
    std::cout<<"try to send msg"<<std::endl;
    for(int i=0;i<100;i++){
        auto st=star(i);
        c->sendmsg_async(st+"~basic~"+std::to_string(i)+st, [](int flag){
            if(flag>=0){
                std::cout<<flag<<" bytes written"<<std::endl;
            }else{
                std::cout<<"Async failure, err="<<flag<<std::endl;
            }
        });
        std::cout<<"msg"<<i<<" has been scheduled to be sent"<<std::endl;
    }
    while(true){} // If we don't wait here, 
}

int main() {
    async_100();
    while(true){}
    return 0;
}

