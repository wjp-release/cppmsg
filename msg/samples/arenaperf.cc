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

static uint8_t tmpbuf[12345]; 

static void init_tmpbuf(){
    for(int i=0;i<12345;){
        string x="hello"+to_string(i);
        memcpy(tmpbuf+i,x.data(),x.size());
        i+=x.size();
    }
    logdebug("Now we have tmpbuf filled with meaningful text.");
}

static const char* hello="hello";

#define nr_msg_parts 500000
#define nr_forge 30

static void new_forge(){
    double avgtime=0;
    int nr=nr_forge;
    for(int i=0;i<nr;i++){
        auto startx = now();
        message x; 
        for(int i=0;i<nr_msg_parts;i++){
            x.append((const uint8_t*)hello, 5);
        }
        auto e=ms_elapsed(startx);
        logdebug("new_forge takes once %f ms", e);
        avgtime+=e;
    }
    avgtime/=nr;
    logdebug("new_forge takes %f ms in avg", avgtime);
}   

static void arena_forge(){
    // arena_pool::instance(); // init pool
    double avgtime=0;
    int nr=nr_forge;
    for(int i=0;i<nr;i++){
        auto startx = now();
        message x = message::create_message_for_recv(nr_msg_parts);
        for(int i=0;i<nr_msg_parts;i++){
            x.append((const uint8_t*)hello, 5);
        }
        avgtime+=ms_elapsed(startx);
    }
    avgtime/=nr;
    logdebug("new_forge takes %f ms in avg", avgtime);
}   

static void new_forge_once(){
    auto start = now();
{
    message x; 
    for(int i=0;i<nr_msg_parts;i++){
        x.append((const uint8_t*)hello, 5);
    }
}
    auto elapsed=ms_elapsed(start);
    logdebug("new_forge takes %d ms", elapsed);
}   

static void arena_forge_once(){
    //arena_pool::instance(); // init pool
    auto start = now();
{
    message x = message::create_message_for_recv(nr_msg_parts);
    for(int i=0;i<nr_msg_parts;i++){
        x.append((const uint8_t*)hello, 5);
    }
}
    auto elapsed=ms_elapsed(start);
    logdebug("arena_forge takes %d ms", elapsed);
}   

#ifdef ENABLE_ARENA
static void try_alloc(){
    arena* a=new arena();
    auto start=now_spec();
    for(int i=0;i<100;i++) auto x=a->alloc(1024);
    auto diff=ns_elapsed(start);
    std::cout<<"try alloc "<<diff.first<<"s "<<diff.second<<"ns elapsed!"<<std::endl;
}
static void try_new(){
    arena* a=new arena();
    auto start=now_spec();
    for(int i=0;i<100;i++) auto x=new uint8_t[1024];
    auto diff=ns_elapsed(start);
    std::cout<<"try new "<<diff.first<<"s "<<diff.second<<"ns elapsed!"<<std::endl;
}
#endif

// Conclusion: arena cannot outperform modern new/malloc by large margin, at least for single-threaded programs. 

int main() {  
    // 
    while(true){}
    return 0;
}
