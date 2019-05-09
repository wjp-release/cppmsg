#pragma once

#include "def.h"
#include "channel/connection.h"
#include "system/addr.h"
#include <unordered_map>

namespace msg{

// Synchronous establishment of connections for debugging or simple scenarios.
// synchronous socket, connect
status sync_connect(const addr& address, int& newfd);
// socket, bind, listen
status bind_listen(const addr& address, int& listenfd);
// synchronous accept 
status sync_accept(int listenfd, int& newfd);

/* 
    basic session maintains one or multiple connections. 
*/
class session{
public:
    template <class T>
    std::shared_ptr<T> create_connection(int fd){
        try{
            auto t=std::make_shared<T>(fd);
            connections[fd]=t;
            return t;
        }catch(...){
            logerr("create connection failed");
            return nullptr;
        }
    }

protected:
    std::unordered_map<int, std::shared_ptr<connection>> connections;  // fd, connection map
};

}