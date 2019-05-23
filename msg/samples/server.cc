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
#include "channel/basic_connection.h"
#include "session/session.h"
using namespace std;
using namespace msg::sample;
using namespace msg;

// server
// unix domain socket event
// timeout event : every 1s write something to client

#define readbuf_size 1024

static std::string star(int n){
    std::string x="";
    for(int i=0;i<n;i+=10)x+="*";
    return x;
}

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
    int listenfd;
    auto s=bind_listen(t->parsed_address,listenfd);
    std::cout<<"listen "<<s.str()<<std::endl;
    int connfd;
    s=sync_accept(listenfd, connfd);
    std::cout<<"accept "<<s.str()<<std::endl;
    auto c=basic_connection::make(connfd);
    message what;
    for(int i=0;i<100;i++){
        std::cout<<"try to recv msg"<<i<<std::endl;
        c->recvmsg(what);
        std::cout<<"msg"<<i<<" recved!"<<std::endl;
        what.print();
        std::cout<<"try to send msg"<<i<<"'s reply"<<std::endl;
        c->sendmsg("Server response <"+std::to_string(i)+">");
        std::cout<<"msg"<<i<<"'s reply is sent!"<<std::endl;
    }
}

struct readhdr_task : public oneiov_read_task{
    readhdr_task(msg::pipe* p) : oneiov_read_task(tmp, 8, std::weak_ptr<basic_connection>()), p(p)
    {
        uint64_t duh=12345;
        memcpy(tmp, &duh, 8);
        auto len=get_following_msglen();
        logdebug("before recv, readhdr len=%d", len);
    }
    msg::pipe* p;
    char tmp[8];
    uint64_t get_following_msglen(){
        return *reinterpret_cast<uint64_t*>(tmp);
    }
    void on_success(int bytes, std::unique_lock<std::mutex>& lk);
    void on_recoverable_failure(int backoff){
        logerr("readhdr_task failed");
        exit(-1);
    }
};

struct readmsg_task : public oneiov_read_task{
    readmsg_task(msg::pipe* p, int size) : oneiov_read_task(0, size, std::weak_ptr<basic_connection>()), p(p){
        tmp = new char [size];
        v.iov_base = tmp;
    }
    ~readmsg_task(){
        if(tmp) delete [] tmp;
    }
    msg::pipe* p;
    char* tmp=0;
    void on_success(int bytes, std::unique_lock<std::mutex>& lk){
        logdebug("%d bytes read: %s", bytes, std::string(tmp, bytes).c_str());
        //p->add_read(std::make_shared<readhdr_task>(p));
        logdebug("now we add a readhdr task");
    }
    void on_recoverable_failure(int backoff){
        logerr("readmsg_task failed");
        exit(-1);
    }
};

void readhdr_task::on_success(int bytes, std::unique_lock<std::mutex>& lk){
    int len= get_following_msglen();
    logdebug("%d bytes read, following msglen=%d", bytes, len);
    if(len>0&&len<4096){
        p->add_read(std::make_shared<readmsg_task>(p, (int)len));
        logdebug("now we add a readmsg task, expect len=%d", len);
    }else{
        logerr("duh, len %d is not valid", len);
    }
}


void simple_conn_server(){
    // init
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
    // conn
    struct readmsg_task;
    msg::pipe c(connfd);
    std::cout<<"pipe created"<<std::endl;
    auto t1=std::make_shared<readhdr_task>(&c);
    c.add_read(t1);
    std::cout<<"the first read task added"<<std::endl;
    while(true){}
}

void basic_server(){
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
    message what;
    for(uint64_t i=0;i<100;i++){
        std::cout<<"try to recv msg"<<i<<std::endl;
        c->recvmsg(what);
        std::cout<<"msg"<<i<<" recved: "<<what.str()<<std::endl;
        what.print();
    }
}

void basic_server_newmsg(){
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

void recvmulti_server(){
    reactor::instance().start_eventloop();
    addr address;
    auto ss=resolv_ipv4_sync(address, "localhost", 12345, 1000);
    if(!ss.is_success()){
        std::cout<<ss.str().c_str()<<std::endl;
        exit(-1);
    }
    std::cout<<"now we have parsed addr"<<std::endl;
    int listenfd;
    auto s=bind_listen(address, listenfd);
    std::cout<<"listen "<<s.str()<<", listenfd="<<listenfd<<std::endl;
    int connfd;
    s=sync_accept(listenfd, connfd);
    std::cout<<"accept "<<s.str()<<", connfd="<<connfd<<std::endl;
    auto c=basic_connection::make(connfd);
    for(uint64_t i=0;i<100;i++){
        message what;
        std::cout<<"try to recv multi msg"<<i<<std::endl;
        c->recv_multipart_msg(what);
        std::cout<<"msg"<<i<<" recved! ";
        what.print();
    }
}

void test_message(){
    message what;
    const char* duh="~~basic~~~";
    for(int i=0;i<1000;i++){
        std::string tmp=star(i)+duh+std::to_string(i)+"!";
        what.append((const uint8_t*)tmp.data(), tmp.size());
    }
    what.print();
}
                           
int main() {  
    recvmulti_server();
    while(true){}
    return 0;
}
