#pragma once

#include "def.h"
#include "ack_connection.h"

namespace msg{ 

/*
    Application level checksum:  
    In practice, the checksum is being asked to detect an error every few thousand packets. After eliminating those errors that the checksum always catches, the data suggests that, on average, between one packet in 10 billion and one packet in a few millions will have an error that goes undetected. The exact range depends on the type of data transferred and the path being traversed. While these odds seem large, they do not encourage complacency. In every trace, one or two 'bad apple' hosts or paths are responsible for a huge proportion of the errors. For applications which stumble across one of the `bad-apple' hosts, the expected time until a corrupted data is accepted could be as low as a few minutes. When compared to undetected error rates for local I/O (e.g., disk drives), these rates are disturbing. Our conclusion is that vital applications should strongly consider augmenting the TCP checksum with an application sum.
*/

// recvmsg sends back an ack after checksum the msg.
// sendmsg sends a msg with checksum info.
class checksum_connection : public ack_connection{
public:
    checksum_connection(int fd):ack_connection(fd){}
    virtual ~checksum_connection(){}
    status disable_checksum(){
        return status::success();
    }
};


}