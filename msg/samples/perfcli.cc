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
using namespace msg;

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

