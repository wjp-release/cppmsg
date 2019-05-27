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

/*
    Batch send & recv: 
    We will repeatedly create and free large messages to simulate batch data transmission.

    Many send & multipart recv : 
    We will append many small message parts to build large messages to simulate large numbers of small messages.
*/
//12KB 
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

static void new_forge(){
    auto start = now();
    for(int i=0;i<30;i++){
        message x; 
        for(int i=0;i<100000;i++){
            x.append((const uint8_t*)hello, 5);
        }
    }
    auto elapsed=ms_elapsed(start);
    logdebug("new_forge takes %d ms", elapsed);
}   

static void arena_forge(){
    arena_pool::instance(); // init pool
    auto start = now();
    for(int i=0;i<30;i++){
        message x = message::create_message_for_recv(50000);
        for(int i=0;i<100000;i++){
            x.append((const uint8_t*)hello, 5);
        }
    }
    auto elapsed=ms_elapsed(start);
    logdebug("arena_forge takes %d ms", elapsed);
}   

// 169 ms
static void new_forge_once(){
    auto start = now();
{
    message x; 
    for(int i=0;i<500000;i++){
        x.append((const uint8_t*)hello, 5);
    }
}
    auto elapsed=ms_elapsed(start);
    logdebug("new_forge takes %d ms", elapsed);
}   

// 169ms --> 64 ms; significant improvement for recv large number of small messages
static void arena_forge_once(){
    arena_pool::instance(); // init pool
    auto start = now();
{
    message x = message::create_message_for_recv(50000);
    for(int i=0;i<500000;i++){
        x.append((const uint8_t*)hello, 5);
    }
}
    auto elapsed=ms_elapsed(start);
    logdebug("arena_forge takes %d ms", elapsed);
}   

int main() {  
    new_forge();
    arena_forge();
    while(true){}
    return 0;
}
