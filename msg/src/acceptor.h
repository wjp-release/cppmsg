#pragma once

#include "defs.h"
#include "protocol.h"

namespace msg{

class Acceptor{
public:
    static std::shared_ptr<Acceptor> create(ProtocolType p, const std::string& url, int port){
        return std::make_shared<Acceptor>(p,url,port);
    }
    void start(){

    }
private:
    Acceptor(ProtocolType p, const std::string& url, int port){

    }
};

}