#pragma once
#include "def.h"
#include "system/addr.h"
#include "common/blockable.h"

#define HintTCP 0
#define HintUDP 1
#define HintActive  0 // connect
#define HintPassive 1 // listen

namespace msg{

// sync mode: do not set cb, call wait() to block until success
// async mode: do not call wait(), set cb.
struct resolv_task : public blockable{
    resolv_task(uint8_t family, uint16_t port, const std::string& hostname, uint8_t passive, uint8_t hint_protocol, resolv_cb cb);
	sa_family_t  family; //parsed from family_v4/v6/uds
    uint16_t     port; //after htons
	std::string  hostname; //raw input
	addr         parsed_address;
    uint8_t      hint_passive;
    uint8_t      hint_protocol;
    resolv_cb    cb; 
    bool         syncmode(){
        return cb==nullptr;
    }
};

class resolv_taskpool {
public:
    ~resolv_taskpool();
    resolv_taskpool() = delete;
    resolv_taskpool(resolv_taskpool &&) = default;
    static resolv_taskpool& instance();
    std::shared_ptr<resolv_task> create_resolv_task(uint8_t family, uint16_t port, const std::string& hostname, uint8_t hint_passive, uint8_t hint_protocol=HintTCP, resolv_cb cb=nullptr);
private:
    struct meta;
    explicit resolv_taskpool(size_t nr_thread);
    std::shared_ptr<meta> meta_;
};

}