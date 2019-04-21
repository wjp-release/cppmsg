#include "message.h"

namespace msg{namespace protocol{

message::message(const std::string& data){
    append((const uint8_t*)data.data(), data.size());
}

void message::append(const uint8_t* data, uint32_t size){
    total_size+=size;
    chunks.emplace_back(data,size);
}

void* message::alloc(uint32_t size){
    total_size+=size;
    chunks.emplace_back(size);
    return reinterpret_cast<void*>(chunks.back().data);
}

void message::append_iov(std::vector<iovec>& iov)const noexcept{
    for(auto& c:chunks){
        iov.push_back({(void*)c.data, (size_t)c.size});
    }
}

void message::set_iov(std::vector<iovec>& iov)const noexcept{
    iov.clear();
    append_iov(iov);
}



}}