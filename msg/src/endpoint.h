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

private:
    std::unordered_map<addr, connection> connections; 
};


}