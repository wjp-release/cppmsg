#pragma once
#include "api/connector.h"
#include "api/connection.h"
#include "api/listener.h"
#include "tcp_connector.h" // created on connector ctor 
#include "tcp_listener.h" // created on listener ctor
#include "tcp_connection.h" // created on accept/connect

namespace msg{namespace posix{ namespace proto{

// [message format] [iov0, iov1, iov2, iov3...] 
// iov0 [raw header] iov1 [msg1] iov2 [msg2]  
// raw header: [iov1len] fix-sized: 64 bits
// We can alloc iov1 once raw header is read.
// The first 64 bits of iov1 stores size of iov2. 
// So after reading iov1, we can then schedule a read request of iov2 to the tcpimpl::conn.
// A message is either fully transferred or dropped. 


// Note that msg::posix::proto::tcp is an application level message protocol built upon oneshot nonblocking reactor and tcp. Don't confuse it with actual tcp though. It's called tcp only because tcp is the changable part in the protocol, which could be replaced by unix domain socket or udp. 

// Note that you should never create a stand-alone tcp object. It is designed to be a crtp(curiously recurring template pattern) template parameter taken by the msg::api classes. 

class tcp : public  msg::api::connector<tcp>, 
                    msg::api::listener<tcp>, 
                    msg::api::connection<tcp>
{
public:
    // three sets of functions
    


private:
    void* data;  //underlying data is tcp_connector*, tcp_connection*, or tcp_listener*.
};


}}}