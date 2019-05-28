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
        claim_memory(arena_pool_size);
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



}