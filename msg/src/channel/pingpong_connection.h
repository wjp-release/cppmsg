#pragma once

#include "def.h"
#include "checksum_connection.h"

namespace msg{ 

/*
    A pingpong_connection is the heaviest connection. 
    It has all the features of the connection family.
*/

class pingpong_connection : public checksum_connection{
    pingpong_connection(int fd):checksum_connection(fd){}
    virtual ~pingpong_connection(){}

};

}