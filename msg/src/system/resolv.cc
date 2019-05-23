#include "resolv.h"

#include <arpa/inet.h>
#include <ctype.h>
#include <netdb.h>
#include <netinet/in.h>
#include <condition_variable>
#include <queue>
#include <thread>
#include "addr.h"
#include "common/taskpool.h"

// We use a single resolver taskq - but we allocate 
namespace msg{
 
#define NrThreads 2

resolv_task::resolv_task(uint8_t family, uint16_t port, const std::string& hostname, uint8_t passive, uint8_t hint_protocol, resolv_cb cb):
        port(htons(port)), // little->big endian
        hostname(hostname), 
        hint_passive(passive),
        hint_protocol(hint_protocol),
        cb(cb)
{
    sa_family_t fam;
    auto s=get_posix_family(family, fam);
    if(!s.is_success()) throw std::runtime_error("invalid family");
    family=fam;
}

resolv_taskpool& resolv_taskpool::instance(){
    static resolv_taskpool tp(NrThreads); 
    return tp;
}

struct resolv_taskpool::meta {
    std::mutex mtx_;
    std::condition_variable cond_;
    bool is_shutdown_ = false;
    std::queue<std::shared_ptr<resolv_task>> tasks_;
};

static status resolv(std::shared_ptr<resolv_task> t){
    // init hint
	addrinfo hints;
	memset(&hints, 0, sizeof(hints));
    hints.ai_family = t->family;
	hints.ai_flags = AI_ADDRCONFIG | AI_NUMERICSERV;
	if(t->hint_passive==HintPassive){
        hints.ai_flags |= AI_PASSIVE;
    } 
    if(t->hint_protocol==HintTCP){
        hints.ai_protocol = IPPROTO_TCP;
        hints.ai_socktype = SOCK_STREAM;
    }else{
        hints.ai_protocol = IPPROTO_UDP;
        hints.ai_socktype = SOCK_DGRAM;
    }
    // getaddrinfo, which is time consuming    
	addrinfo *results = nullptr;
	if(getaddrinfo(t->hostname.c_str(), "123", &hints, &results) != 0) {
        if (results) freeaddrinfo(results);
        return status::failure("getaddrinfo failed");
	}
    // fetch the first valid result, if any
    auto p = results;
	for(; p; p = p->ai_next) {
		if(p->ai_addr->sa_family == AF_INET) {
            sockaddr_in * sin=(sockaddr_in*)p->ai_addr;
            t->parsed_address.init(family_v4, t->port, sin);
            break;
		}else if(p->ai_addr->sa_family==AF_INET6){
		    sockaddr_in6 *sin6=(sockaddr_in6*)p->ai_addr;
            t->parsed_address.init(family_v4, t->port, sin6);
            break;
        }
	}
    if (results) freeaddrinfo(results);
    if(p) return status::success();
    else return status::failure("no valid result");
}

std::shared_ptr<resolv_task> resolv_taskpool::create_resolv_task(uint8_t family, uint16_t port, const std::string& hostname, uint8_t hint_passive, uint8_t hint_protocol, resolv_cb cb)
{
    try{
	    auto t=std::make_shared<resolv_task>(family, port, hostname, hint_passive, hint_protocol, cb);
        {
            std::lock_guard<std::mutex> lk(meta_->mtx_);
            meta_->tasks_.emplace(t);
        }
        meta_->cond_.notify_one();
        return t;
    }catch(...){
        return nullptr;
    }
}


resolv_taskpool::resolv_taskpool(size_t nr_thread) : meta_(std::make_shared<meta>()) {
    for (size_t i = 0; i < nr_thread; i++) {
        std::thread([this]{
            std::unique_lock<std::mutex> lk(meta_->mtx_);
            while(!meta_->is_shutdown_) {
                if (!meta_->tasks_.empty()) {
                    auto curtask = std::move(meta_->tasks_.front());
                    addr a=curtask->parsed_address;
                    resolv_cb cb=curtask->cb; 
                    meta_->tasks_.pop();
                    lk.unlock();
                    resolv(curtask);
                    if(curtask->syncmode()){
                        curtask->signal(); 
                    }else{ // capture by value 
                        taskpool::instance().execute([a, cb]{
                            cb(a); 
                        });
                    }
                    lk.lock();
                } else {
                    meta_->cond_.wait(lk);
                }
            }
        }).detach();
    }
}

resolv_taskpool::~resolv_taskpool() {
    if ((bool) meta_) {
        {
            std::lock_guard<std::mutex> lk(meta_->mtx_);
            meta_->is_shutdown_ = true;
        }
        meta_->cond_.notify_all();
    }
}

status resolv_ipv4_sync(addr& address, const char* host, int port, int ms_timeout){
    auto t=resolv_taskpool::instance().create_resolv_task(family_v4,port,host,1, 0, 0);
    if(ms_timeout==0){
        t->wait();
        address=t->parsed_address;
        return status::success();
    }else{
        bool rc=t->wait_for(ms_timeout);
        address=t->parsed_address;
        if(rc){
            return status::success();
        }else{
            return status::failure("resolv timeout");
        }
    }
}

status resolv_ipv6_sync(addr& address, const char* host, int port, int ms_timeout){
    auto t=resolv_taskpool::instance().create_resolv_task(family_v6,port,host,1, 0, 0);
    if(ms_timeout==0){
        t->wait();
        address=t->parsed_address;
        return status::success();
    }else{
        bool rc=t->wait_for(ms_timeout);
        address=t->parsed_address;
        if(rc){
            return status::success();
        }else{
            return status::failure("resolv timeout");
        }
    }
}


}