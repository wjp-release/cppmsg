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
    void async_connect_quietly();
    void async_connect(async_cb on_connected);

private:
    std::unordered_map<int, std::shared_ptr<connection>> connections;  // fd, connection map
};


}