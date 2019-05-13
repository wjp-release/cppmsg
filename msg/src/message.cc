#include "message.h"

namespace msg{

message::message(const std::string& data){
    append((const uint8_t*)data.data(), data.size());
}

message::message(const char* data){
    append((const uint8_t*)data, strlen(data));
}

message::message(const uint8_t* data, uint32_t size){
    append(data, size); 
}

std::string message::str(){
    std::string s;
    for(auto& c:chunks){
        s+=std::string((char*)c.data,c.size);
    }
    return s;
}

void message::append(const uint8_t* data, uint32_t size){
    assert(size<=MaxSize);
    total_size+=size;
    chunks.emplace_back(data,size);
}

void* message::alloc(uint32_t size){
    assert(size<=MaxSize);
    total_size+=size;
    chunks.emplace_back(size);
    return reinterpret_cast<void*>(chunks.back().data);
}

void message::append_to_iovs(std::vector<iovec>& iov)const noexcept{
    for(auto& c:chunks){
        iov.push_back({(void*)c.data, (size_t)c.size});
    }
}

void message::convert_to_iovs(std::vector<iovec>& iov)const noexcept{
    iov.clear();
    append_to_iovs(iov);
}



}