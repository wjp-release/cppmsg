#pragma once

#include "def.h"
#include "channel/connection.h"
#include "system/addr.h"
#include <unordered_map>

/*
    An enpoint maintains connections with one or multiple peers.
*/

namespace msg{

class endpoint{
public:
    // async connect; will retry until success
    status async_connect_quietly(){

    }
    status async_connect(async_cb on_connected){

    }
    // block: synchronous socket, connect
    status connect(const addr& address, int& newfd);
    // fast: socket, bind, listen
    status listen(const addr& address, int& listenfd);
    // block: synchronous accept 
    status accept(int listenfd, int& newfd);

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

private:
    std::unordered_map<int, std::shared_ptr<connection>> connections;  // fd, connection map
};


}