#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "posix/reactor.h"
using namespace std;

void eventloop_test(){
    int duh=0;
    msg::posix::reactor::instance().start_eventloop();
    msg::posix::reactor::instance().run_later([&]{
        cout<<"duh"<<duh++<<endl;
    });
    msg::posix::reactor::instance().run_later([&]{
        cout<<"boo"<<duh++<<endl;
    });
    msg::posix::sleep(100);
    msg::posix::reactor::instance().run_later([&]{
        cout<<"fww"<<duh++<<endl;
    });
    msg::posix::reactor::instance().run_later([&]{
        cout<<"xxx"<<duh++<<endl;
    });
    msg::posix::sleep(100);
    msg::posix::reactor::instance().run([&]{
        cout<<"run"<<endl;
    }); 
}

void signalfd_test(){
    msg::posix::reactor::instance().start_eventloop();
}

void timer_test(){
    msg::posix::reactor::instance().eventloop();
}

int main() {
    timer_test();
    return 0;
}

