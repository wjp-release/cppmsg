#pragma once

#include "def.h"
#include "connection.h"
#include "reconnector.h"
#include "relistener.h"

namespace msg{ namespace protocol{

// An endpoint maintains multiple reconnectors, relisteners, and connections. 
class endpoint{ 
public:
    using reconnptr=std::shared_ptr<reconnector>;
    using relistenptr=std::shared_ptr<relistener>;
    using connptr=std::shared_ptr<connection>;
    static endpoint& instance(){
        static endpoint ep;
        return ep;
    }
    void remove_connection(const connptr& c);
    void create_message_connection(int fd);
    void create_reliable_connection(int fd);
    void create_very_reliable_connection(int fd);
    void create_direct_connection(int fd);
private:
    endpoint(){}
    std::list<reconnptr>    reconnectors; // try to connect
    std::list<relistenptr>  relisteners; // try to accept
    std::list<connptr>      connections; // established connections
};

}}
