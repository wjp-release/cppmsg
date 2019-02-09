#pragma once

#include "defs.h"
#include "message.h"

namespace msg{

/*-----------------------------------------------
 Messaging Patterns
 1. classic publish-subscribe model
 2. asynchronous many-to-many 
 3. synchronous one-to-one
 4. asynchronous many-to-many 
 5. synchronous one-to-(load balanced one in many) 
 6. asynchronous one-to-many
 
 PatternTypes:
 1. pub_server: will never block; may accept many 
                sub_clients
 2. sub_client: will never block; may connect many 
                pub_servers; supports filtering
 3. async_server: will never block; may accept many
                sync/async clients
 4. async_client: will never block; may connect many
                sync/async servers
 5. sync_server:  will block on recv unless it recvs
                a msg; may accpet one client
 6. sync_client: will block on send unless it gets a 
                reply; may connect many servers; but 
                only one in many is load balanced as
                its destination
 7. default: behaves like smarter POSIX sockets
-----------------------------------------------*/

enum class PatternType{
    pub_server,
    sub_client, 
    async_server,  
    async_client,
    sync_server, 
    sync_client, 
    default,
};

class Pattern{
public:
    static std::unique_ptr<Pattern> create(PatternType){
        // todo
        return nullptr;
    }
    void send(Message message);
    Message recv();
private:

};

}