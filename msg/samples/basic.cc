#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "reactor/reactor.h"
#include "common/clock.h"
#include "sample.h"

using namespace std;
using namespace msg::sample;

void eventloop_test(){
    int duh=0;
    msg::reactor::reactor::instance().start_eventloop();
    msg::reactor::reactor::instance().submit([&]{
        cout<<"duh"<<duh++<<endl;
    });
    msg::reactor::reactor::instance().submit([&]{
        cout<<"boo"<<duh++<<endl;
    });
    msg::common::sleep(100);
    msg::reactor::reactor::instance().submit([&]{
        cout<<"fww"<<duh++<<endl;
    });
    msg::reactor::reactor::instance().submit([&]{
        cout<<"xxx"<<duh++<<endl;
    });
    msg::common::sleep(100);
    msg::reactor::reactor::instance().submit_and_wake([&]{
        cout<<"run"<<endl;
    }); 
}

void signalfd_test(){
    msg::reactor::reactor::instance().start_eventloop();
}

void timer_test(){
    msg::reactor::reactor::instance().eventloop();
}

void tcp_test(){

}

int main() {
    eventloop_test();
    return 0;
}

