#pragma once

#include "defs.h"
#include "message.h"

namespace msg{

enum class ProtocolType{
    tcp,
    ipc,
    inproc,
    websocket
};

class Protocol{
public:
    Protocol()=delete;
    std::unique_ptr<Protocol> create(ProtocolType p){
        //todo 
        return nullptr;
    }
    void send(Message message);
    Message recv();
};

}