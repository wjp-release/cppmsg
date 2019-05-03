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
    // synchronous socket, connect
    status connect(const addr& address, int& newfd);
    // synchronous socket, bind, listen, accept
    status accept(const addr& address, int& newfd);

private:
    std::unordered_map<int, std::shared_ptr<connection>> connections;  // fd, connection map
};


}