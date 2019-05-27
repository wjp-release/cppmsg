#pragma once

#include "def.h"
#include <condition_variable>

namespace msg{ 

// Efficient and cache-friendly memory allocation for building very large messages. 
class arena{
protected:
    //According to my experiment, pointers that needs alignment is around 13% of all pointers. 
    inline uint32_t align_offset(){
        uint32_t off = 8-(((uint64_t)curpos)&7);
        if(off==8) off = 0;
        return off;
    }
    inline uint32_t remaining_bytes(){
        return arena_chunk_size-(curpos-curchunk);
    } 
    uint8_t* create_chunk(uint32_t bytes){
        auto c=new uint8_t[bytes];
        arena_chunks.push_back(c);
        return c;
    }
    uint8_t* alloc_overflow(uint32_t bytes){
        curchunk=create_chunk(arena_chunk_size);
        curpos=curchunk+bytes; 
        return curchunk;
    }
public:
    arena(){
        curchunk=create_chunk(arena_chunk_size);
        curpos=curchunk;
    }
    arena(const arena&) = delete;
    arena& operator=(const arena&) = delete;
    arena(arena&&) = delete;
    arena& operator=(arena&&) = delete;
    ~arena(){
        for(auto c : arena_chunks){
            delete[]c;
        }
    }
    // reset state to newly constructed 
    void reset(){
        for(auto i=std::next(arena_chunks.begin());i!=arena_chunks.end();i++){
            delete [] (*i);
        }
        curchunk=*arena_chunks.begin();
        curpos=curchunk;
    }
    uint8_t* alloc(uint32_t bytes){
        if(bytes>=arena_chunk_size){
            return create_chunk(bytes);
        }
        uint32_t aligned_bytes=bytes+align_offset();
        if(aligned_bytes <= remaining_bytes()){
            curpos += aligned_bytes;
            return curpos-bytes;
        }else{
            return alloc_overflow(bytes);
        }
    }
private:
    uint8_t* curpos = 0;
    uint8_t* curchunk = 0;
    std::list<uint8_t*> arena_chunks; //overflow list
};

//Thread-safe arena pool for faster arena acquisition and background arena recycling.
class arena_pool{
protected:
    void claim_memory(int poolsize){
        std::lock_guard<std::mutex> lk(freemtx);
        for(int i=0;i<poolsize;i++){
            freelist.push_back(new arena{});
        }
    }
    void release_memory(){
        // release unreferred arenas
        {
            std::lock_guard<std::mutex> lk(freemtx);
            for(auto a:freelist) delete a;
            freelist.clear();
        }
        {
            std::lock_guard<std::mutex> lk(reapmtx);
            for(auto a:reaplist) delete a;
            reaplist.clear();
        }
    }
    void recycle(arena* a){
        a->reset(); // The reaper thread is the sole owner of a when recycle() is called. There is no race condition here.
        {
            std::lock_guard<std::mutex> lk(freemtx);
            freelist.push_back(a);
        }
    }
public:
    arena_pool(){
        // A background thread that recycles arenas
        std::thread([this]{
            std::unique_lock<std::mutex> lk(reapmtx);
            while(true){
                if(reaplist.empty()){
                    reapcv.wait(lk);
                }else{
                    auto a=reaplist.back();
                    reaplist.pop_back();
                    lk.unlock(); // now it's possible to insert new reaped arenas
                    recycle(a); 
                    lk.lock(); 
                }
            }
        }).detach();
    }
    static arena_pool& instance(){
        static arena_pool pool;
        return pool;
    }
    // Enabling arena pooling consumes 4MB memory.
    void enable(int poolsize=arena_pool_size){
        std::lock_guard<std::mutex> lk(enabledmtx);
        if(enabled) return;
        enabled=true;
        claim_memory(poolsize);
    }
    // Disabling arena pooling releases 4MB memory. 
    void disable(){
        std::lock_guard<std::mutex> lk(enabledmtx);
        if(!enabled) return;
        enabled=false;
        release_memory();
    }
    bool exists_free_arena(){
        std::lock_guard<std::mutex> lk(freemtx);
        return !freelist.empty(); 
    }
    // Note that ref() still works even if pooling disabled.
    arena* ref(){
        arena* a;
        {
            std::lock_guard<std::mutex> lk(freemtx);
            if(!freelist.empty()){
                a=freelist.back();
                freelist.pop_back();
            }else{
                a=new arena(); 
            }
        }
        return a;
    }
    // Note that deref() does not recycle arenas if pooling disabled. 
    void deref(arena* a){
        {
            std::lock_guard<std::mutex> lk(enabledmtx);
            if(!enabled){
                delete a;
                return;
            }
        }
        {
            std::lock_guard<std::mutex> lk(reapmtx);
            reaplist.push_back(a);    
        }
        reapcv.notify_all();
    }
private:
    mutable std::mutex enabledmtx;
    bool              enabled=false;
    mutable std::mutex freemtx;
    std::list<arena*> freelist;
    mutable std::mutex reapmtx;
    mutable std::condition_variable reapcv;
    std::list<arena*> reaplist; 
};


// A message_chunk a single unit of message data container.
struct message_chunk{
    message_chunk(){}
    message_chunk& operator=(const message_chunk& x)=delete;
    message_chunk& operator=(message_chunk&& x)=delete;
    message_chunk(message_chunk&& x)=delete;
    message_chunk(const message_chunk& x)=delete;
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
        if(a){ // deref arena for large messages
            arena_pool::instance().deref(a);
        }else{ // free data one by one for small ones
            for(auto& c : chunks){
                delete [] c.data;
            }
        }
    }
    void use_arena(){ //for building large messages
        if(!a) a = arena_pool::instance().ref();
    }
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
    uint32_t    total_size=0;
    arena*      a=0; 
    std::list<message_chunk> chunks; // vector will cause stupid copy reallocation
};


// Note that message objects can be copied, stored and passed as an integral value, without causing replication of its underlying message_meta object. 
// It also ensures even if client mistakenly destroys the message which is taken as recvmsg() function's argument, the function can still function properly without referring to an already destroyed object.
class message{
public:
    // Create messages for recvmsg() via this method.
    static message create_message_for_recv(int expected_size=0){
        message msg;
        if(expected_size>=large_message_size){
            msg.use_arena();
        }
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
    void use_arena(){ //for building large messages
        meta->use_arena();
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