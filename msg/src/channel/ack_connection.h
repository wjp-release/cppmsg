#pragma once

#include "def.h"
#include "healthcheck_connection.h"

namespace msg{ 

/*
    An ack connection implements application level ACK(acknowledgement) and ARQ(automatic repeated request).
*/

class ack_connection : public healthcheck_connection{
public:
    ack_connection(int fd):healthcheck_connection(fd){}
    virtual ~ack_connection(){}

private:

};


}