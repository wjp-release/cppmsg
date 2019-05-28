#pragma once

#include "def.h"

#ifdef ENABLE_ARENA
#include "common/arena.h"
#endif

namespace msg{ 

// A message_chunk a single unit of message data container.
struct message_chunk{
    message_chunk(){}
    message_chunk& operator=(const message_chunk& x)=delete;
    message_chunk& operator=(message_chunk&& x)=delete;
    message_chunk(message_chunk&& x)=delete;
    message_chunk(const message_chunk& x)=delete;
    #ifdef ENABLE_ARENA
    // After creating empty chunks, recvmsg will immediately fill it with exact size bytes
    message_chunk(uint32_t size, arena* a):size(size){
        if(a) data=a->alloc(size);
        else data=new uint8_t[size];
    }
    message_chunk(const uint8_t* d, uint32_t size, arena* a):size(size){
        if(a) data=a->alloc(size);
        else data=new uint8_t[size];
        memcpy(data, d, size);
    }
    #else
    message_chunk(uint32_t size):size(size){
        data=new uint8_t[size];
    }
    message_chunk(const uint8_t* d, uint32_t size):size(size){
        data=new uint8_t[size];
        memcpy(data, d, size);
    }
    #endif
    ~message_chunk(){ 
        //logdebug("message chunk released!");
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
    ~message_meta(){
        #ifdef ENABLE_ARENA
        if(a){ // deref arena for large messages
            #ifdef ENABLE_ARENA_POOLING
            arena_pool::instance().deref(a);
            #else
            delete a;
            #endif
        }else{ // free data one by one for small ones
            for(auto& c : chunks){
                delete [] c.data;
            }
        }
        #else
        for(auto&c:chunks) delete [] c.data;
        #endif
    }
    #ifdef ENABLE_ARENA
    void use_arena(){ //for building large messages
        #ifdef ENABLE_ARENA_POOLING
        if(!a) a = arena_pool::instance().ref();
        #else
        if(!a) a = new arena();
        #endif
    }
    #endif
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
        #ifdef ENABLE_ARENA
        a->reset();
        #endif
    }
    void append_to_iovs(std::vector<iovec>& iov)const noexcept;
    void convert_to_iovs(std::vector<iovec>& iov)const noexcept;
    uint32_t    total_size=0;
    #ifdef ENABLE_ARENA
    arena*      a=0; 
    #endif
    std::list<message_chunk> chunks; // vector will cause stupid copy reallocation
};


// Note that message objects can be copied, stored and passed as an integral value, without causing replication of its underlying message_meta object. 
// It also ensures even if client mistakenly destroys the message which is taken as recvmsg() function's argument, the function can still function properly without referring to an already destroyed object.
class message{
public:
    // Create messages for recvmsg() via this method.
    static message create_message_for_recv(int expected_size=0){
        message msg;
        #ifdef ENABLE_ARENA
        if(expected_size>=large_message_size){
            msg.use_arena();
        }
        #endif
        return msg;
    }
    static const int MaxSize=message_meta::MaxSize; //64MB
    message():meta(std::make_shared<message_meta>()){}
    // Create messages to send directly by calling constructors
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
    #ifdef ENABLE_ARENA
    void use_arena(){ //for building large messages
        meta->use_arena();
    }
    #endif
    bool is_reusable(){
        
    }
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