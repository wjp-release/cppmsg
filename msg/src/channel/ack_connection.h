#pragma once

#include "def.h"
#include "basic_connection.h"

namespace msg{ 

// Success of sendmsg is defined as receving an ack from the receiver. 
// sendmsg tries to recv an ack after message_connection::sendmsg.
// recvmsg sends back an ack on a successful message_connection::recvmsg.
class ack_connection : public basic_connection{
public:
    ack_connection(int fd):basic_connection(fd){}
    virtual ~ack_connection(){}

private:

};


}