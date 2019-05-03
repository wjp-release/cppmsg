#pragma once

#include "def.h"
#include "checksum_connection.h"

namespace msg{ 

// sendmsg tries to recv a stateful ack 
// recvmsg sends back an ack if the request has been handled
class pingpong_connection : public checksum_connection{
    pingpong_connection(int fd):checksum_connection(fd){}
    virtual ~pingpong_connection(){}

};

}