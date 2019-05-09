#pragma once

#include "def.h"
#include "logical_session.h"

namespace msg{

/*
    client session could be configured to enable auto-reconnect & heartbeat health check. 
*/ 
class client_session : public logical_session{
public:
    // async connect; will retry until success
    status async_connect_quietly(){

        return status::success();
    }
    status async_connect(async_cb on_connected){

        return status::success();
    }
protected:
};

}