#pragma once

#include "def.h"
#include "transport/conn.h"
#include "message.h"

namespace msg{ namespace protocol{
class endpoint;


// msg protocol level connection, owned by endpoints
class connection{
public:
    using native_conn=std::unique_ptr<msg::transport::conn>;
    connection(endpoint& ep, int fd):owner(ep), conn(std::make_unique<msg::transport::conn>(fd)){

    }
    ~connection(){
        close();
    }
    void            sendmsg(const message& msg){

    }
    void            recvmsg(message& msg){
        
    }
    // close of a connection will trigger on_failure(conn_close) of all its io_tasks
    void            close(){
        conn->close();
    }

private:
    native_conn     conn;  // thread-safe
    uint8_t         head_read_buffer[8]; // an 8-bytes read_task buffer for reading msg headers
    uint8_t         header_writer_buffer[8]; // an 8-bytes write_task buffer for writing msg headers
    endpoint&       owner; 
};


}}