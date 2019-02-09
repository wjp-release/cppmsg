#pragma once

#include "defs.h"
#include "protocol.h"

namespace msg{

class Connector{
public:
    static std::shared_ptr<Connector> create(ProtocolType p, const std::string& url){
        return std::make_shared<Connector>(p,url);
    }
    void startSynchronously(){

    }

private:
    Connector(ProtocolType p, const std::string& url){

    }
};

}