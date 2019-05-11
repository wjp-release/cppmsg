#pragma once

#include "def.h"
#include "basic_connection.h"

namespace msg{ 


/*
    A healthcheck_connection implements periodic heartbeats as means of health check. 

    

    Note that the hearbeats feature is disabled by default. Subclasses of this class should explicitly call enable_heartbeats() to create a heath check timer routine. 
*/

class healthcheck_connection : public basic_connection {
public:
    healthcheck_connection(int fd):basic_connection(fd){}
    virtual ~healthcheck_connection(){}
    void              enable_heartbeats(){
        heartbeats=true;
    }
    void              disable_heartbeats(){
        heartbeats=false;
    }
    
protected:
    bool            heartbeats=false;
};



}