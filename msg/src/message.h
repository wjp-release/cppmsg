#pragma once

#include "def.h"

namespace msg{ 
// Arena needed!
class arena{
public:


private:


};

// A message_chunk a single unit of message data container.
struct message_chunk{
    message_chunk(){}
    message_chunk& operator=(const message_chunk& x)=delete;
    message_chunk& operator=(message_chunk&& x)=delete;
    message_chunk(message_chunk&& x)=delete;
    message_chunk(const message_chunk& x)=delete;
    // After creating empty chunks, recvmsg will immediately fill it with exact size bytes
    message_chunk(uint32_t size):size(size){
        data=new uint8_t[size];
    }
    message_chunk(const uint8_t* d, uint32_t size):size(size){
        data=new uint8_t[size];
        memcpy(data, d, size);
    }
    ~message_chunk(){
        logdebug("message chunk released!");
        if(data) delete [] data;
    }
    uint32_t size=0;
    uint8_t* data=nullptr;
};

class message_meta{
public:
    static const int MaxSize=1024*1024*64; //64MB
    message_meta(){}
    message_meta(const std::string& data);
    message_meta(const char* data);
    message_meta(const uint8_t* data, uint32_t size);
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
            std::cout<<" ["<<std::string((char*)i.data, i.size)<<"]";
        }
        std::cout<<std::endl;
    }
    std::string str() const; 
    bool empty() const noexcept{
        return total_size==0;
    }
    void clear(){
        total_size=0;
        chunks.clear();
    }
    void append_to_iovs(std::vector<iovec>& iov)const noexcept;
    void convert_to_iovs(std::vector<iovec>& iov)const noexcept;
    uint32_t total_size=0;
    std::list<message_chunk> chunks; // vector will cause stupid copy reallocation
};

// Note that message objects can be copied, stored and passed as an integral value, without causing data replication of its underlying message_meta object. 
// It also ensures even if client mistakenly destroys the message which is taken as recvmsg() function's argument, the function can still function properly without referring to an already destroyed object.
class message{
public:
    static const int MaxSize=message_meta::MaxSize; //64MB
    message():meta(std::make_shared<message_meta>()){}
    message(const std::string& data):meta(std::make_shared<message_meta>(data)){}
    message(const char* data):meta(std::make_shared<message_meta>(data)){}
    message(const uint8_t* data, uint32_t size):meta(std::make_shared<message_meta>(data,size)){}
    message(const message& msg) noexcept : meta(msg.meta){}
    message(message&& msg) noexcept{meta.swap(msg.meta);}
    virtual ~message(){}
    message& operator=(message&&) noexcept;
    message& operator=(const message&) noexcept;
    bool operator==(const message& other) const noexcept;
    bool operator!=(const message& other) const noexcept;
    void append(const uint8_t* data, uint32_t size){
        meta->append(data,size);
    }
    void* alloc(uint32_t size){
        return meta->alloc(size);
    }
    int nr_chunks()const noexcept{
        return meta->nr_chunks();
    }
    int size()const noexcept{
        return meta->total_size;
    }
    void print(){ //debug
        meta->print();
    }
    std::string str()const{
        return meta->str();
    }
    bool empty() const noexcept{
        return meta->total_size==0;
    }
    void clear(){
        meta->clear();
    }
    void append_to_iovs(std::vector<iovec>& iov)const noexcept{
        meta->append_to_iovs(iov);
    }
    void convert_to_iovs(std::vector<iovec>& iov)const noexcept{
        meta->convert_to_iovs(iov);
    }
private:
    std::shared_ptr<message_meta> meta;
};


}