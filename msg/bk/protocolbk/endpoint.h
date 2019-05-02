#pragma once

#include "def.h"
#include "channel/connection.h"
#include "connect/reconnector.h"
#include "listen/listener.h"

namespace msg{ 

// An endpoint maintains multiple reconnectors, relisteners, and connections. 
class endpoint{ 
public:
    using reconnptr=std::shared_ptr<reconnector>;
    using listenptr=std::shared_ptr<listener>;
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
    std::list<listenptr>    listeners; // try to accept
    std::list<connptr>      connections; // established connections
};

}
