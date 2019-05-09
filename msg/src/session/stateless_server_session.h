#pragma once

#include "def.h"
#include "session.h"

namespace msg{

/*
    stateless_server_session maintains passive connections only.  
    It does not track each connection's peer addr. Whenever a fatal error happens, it just drops the connection. 
    It does not support reconnection from this end, which is usually too costly for server side.
*/
class stateless_server_session : public session{
public:

protected:

};



}