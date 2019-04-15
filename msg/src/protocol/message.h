#pragma once

#include "def.h"

namespace msg{ namespace protocol{

// A message_chunk a single unit of message data container.
struct message_chunk{
    message_chunk(){}
    message_chunk(uint8_t* d, uint32_t size):size(size){
        data=new uint8_t[size];
        memcpy(data, d, size);
    }
    ~message_chunk(){
        if(data&&) delete [] data;
    }
    uint32_t size=0;
    uint8_t* data=nullptr;
};

// A message consists of multiple message_chunks. 
// A message itself is a pure container. To send a multipart message, a networking object should map the message with its number of transferred chunks, and maintain this mapping information until the transfer ends.
class message{
public:
    void append(uint8_t* data, uint32_t size){
        std::lock_guard<std::mutex> lk(mtx);
        chunks.emplace(data,size);
    }
private:
    std::mutex                 mtx;
    std::vector<message_chunk> chunks;
};

}}