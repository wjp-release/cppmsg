#include "message.h"

namespace msg{

message_meta::message_meta(const std::string& data){
    #ifdef ENABLE_ARENA
    if(data.size()>large_message_size){
        use_arena();
    } 
    #endif
    append((const uint8_t*)data.data(), data.size());
}

message_meta::message_meta(const char* data){
    uint32_t datalen=strlen(data);
    #ifdef ENABLE_ARENA
    if(datalen>large_message_size){
        use_arena();
    } 
    #endif
    append((const uint8_t*)data, datalen);
}

message_meta::message_meta(const uint8_t* data, uint32_t size){
    #ifdef ENABLE_ARENA
    if(size>large_message_size){
        use_arena();
    } 
    #endif
    append(data, size); 
}

std::string message_meta::str()const{
    std::string s;
    for(auto& c:chunks){
        s+=std::string((char*)c.data,c.size);
    }
    return s;
}

void message_meta::append(const uint8_t* data, uint32_t size){
    assert(size<=MaxSize);
    total_size+=size;
    #ifdef ENABLE_ARENA
    chunks.emplace_back(data, size, a);
    #else
    chunks.emplace_back(data, size);
    #endif
}

void* message_meta::alloc(uint32_t size){
    assert(size<=MaxSize);
    total_size+=size;
    #ifdef ENABLE_ARENA
    chunks.emplace_back(size, a);
    #else
    chunks.emplace_back(size);
    #endif
    return reinterpret_cast<void*>(chunks.back().data);
}

void message_meta::append_to_iovs(std::vector<iovec>& iov)const noexcept{
    for(auto& c:chunks){
        iov.push_back({(void*)c.data, (size_t)c.size});
    }
}

void message_meta::convert_to_iovs(std::vector<iovec>& iov)const noexcept{
    iov.clear();
    append_to_iovs(iov);
}

message& message::operator=(message&&msg) noexcept{
    meta.swap(msg.meta);
    return *this;
}

message& message::operator=(const message& msg) noexcept{
    meta=msg.meta;
    return *this;
}

bool message::operator==(const message& other) const noexcept{
    return meta==other.meta;
}

bool message::operator!=(const message& other) const noexcept{
    return !(*this==other);
}


}