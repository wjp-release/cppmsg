#pragma once

#include "def.h"
#include "session.h"

namespace msg{

/*
    A logical session maintains active connections and fd to addr mapping. 
    Physical connections are identified by fd; they would be dropped on fatal error.
    Logical connections are identified by addr; they will always exist even if the underlying physical connection is down. 
*/ 
class logical_session{
public:
    status enable_auto_reconnection(){


        return status::success();
    }
    status enable_heartbeat_health_check(){
        
        return status::success();
    }
protected:
    std::unordered_map<addr, int> addr_fd_map; //fd==-1 represents no connection established yet
};



}