#pragma once

#include "def.h"

namespace msg{ namespace protocol{

// A message_chunk a single unit of message data container.
struct message_chunk{
    message_chunk(){}
    // After creating empty chunks, recvmsg will immediately fill it with exact size bytes
    message_chunk(uint32_t size):size(size){
        data=new uint8_t[size];
        //memset(data, 0, size); // unnecessary to bzero
    }
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

// A message consists of multiple message_chunks. It is not thread-safe.
class message{
public:
    void append(uint8_t* data, uint32_t size){
        total_size+=size;
        chunks.emplace(data,size);
    }
    void* alloc(uint32_t size){
        total_size+=size;
        chunks.emplace(size);
        return reinterpret_cast<void*>(chunks.back().data);
    }
    int nr_chunks()const noexcept{
        return chunks.size();
    }
    int size()const noexcept{
        return total_size;
    }
    void append_iov(vector<iovec>& iov)const noexcept{
        for(auto& c:chunks){
            iov.emplace_back({(void*)c.data, (size_t)c.size});
        }
    }
    void set_iov(vector<iovec>& iov)const noexcept{
        iov.clear();
        append_iov(iov);
    }
private:
    uint32_t total_size=0;
    std::vector<message_chunk> chunks;
};

}}