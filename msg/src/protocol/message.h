#pragma once

#include "def.h"

namespace msg{ namespace protocol{

// A message_chunk a single unit of message data container.
struct message_chunk{
    message_chunk(){}
    // After creating empty chunks, recvmsg will immediately fill it with exact size bytes
    message_chunk(uint32_t size):size(size){
        data=new uint8_t[size];
    }
    message_chunk(const uint8_t* d, uint32_t size):size(size){
        data=new uint8_t[size];
        memcpy(data, d, size);
    }
    ~message_chunk(){
        if(data) delete [] data;
    }
    uint32_t size=0;
    uint8_t* data=nullptr;
};

// A message consists of multiple message_chunks. It is not thread-safe.
class message{
public:
    message(){}
    message(const std::string& data);
    void append(const uint8_t* data, uint32_t size);
    void* alloc(uint32_t size);
    int nr_chunks()const noexcept{
        return chunks.size();
    }
    int size()const noexcept{
        return total_size;
    }
    void print(){ //debug
        std::cout<<"size="<<size()<<", nr_chunks="<<nr_chunks();
        for(auto& i:chunks){
            std::cout<<"["<<std::string((char*)i.data, i.size)<<"] ";
        }
        std::cout<<std::endl;
    }
    void append_iov(std::vector<iovec>& iov)const noexcept;
    void set_iov(std::vector<iovec>& iov)const noexcept;
private:
    uint32_t total_size=0;
    std::vector<message_chunk> chunks;
};

}}