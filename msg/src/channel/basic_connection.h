#pragma once

#include "def.h"
#include "connection.h"

namespace msg{ 

// msg protocol level connection, owned by endpoints
class basic_connection : public connection {
public:
    basic_connection(int fd):connection(fd){}
    virtual ~basic_connection(){}
    virtual status    sendmsg(const message& msg);
    virtual void      sendmsg_async(const message& msg, const async_cb& cb=nullptr){
        //todo 
    }

    virtual status    recvmsg(message& msg);
    virtual void      recvmsg_async(message& msg, const async_cb& cb=nullptr){
        //todo 
    }

    virtual status    recv_multipart_msg(message& msg){



    }

    virtual void      recv_multipart_msg_async(message& msg, const async_cb& cb=nullptr){
        


    }

private:
    char            buf[8]; // hdr buffer
    uint32_t        send_timeout=0; // ms
    uint32_t        recv_timeout=0; // ms
    // todo: options like open/close nagle, send/recv timeouts, etc
    // todo: heartbeats 
};



}