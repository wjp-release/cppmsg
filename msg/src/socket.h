#pragma once

#include "defs.h"
#include "pattern.h"
#include "protocol.h"
#include "connector.h"
#include "acceptor.h"

namespace msg{

/*-----------------------------------------------


-----------------------------------------------*/

class Socket{
public:
    Socket(PatternType p) : pattern(Pattern::create(p)){}
    void accept(ProtocolType p, const std::string& url, int port){
        auto acceptor=Acceptor::create(p,url,port);
        acceptors.push_back(acceptor);
        acceptor->start();
    }
    void connect(ProtocolType p, const std::string& url){
        auto connector=Connector::create(p, url);
        connectors.push_back(connector);
        connector->startSynchronously();
    }
    void send(Message message){
        pattern->send(message);
    }
    Message recv(){
        return pattern->recv();
    }
private:
    std::vector<std::shared_ptr<Connector>> connectors;
    std::vector<std::shared_ptr<Acceptor>> acceptors;
    const int32_t sendTimeout=-1; 
    const int32_t recvTimeout=-1;
    std::unique_ptr<Pattern> pattern;
};

}