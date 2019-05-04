#pragma once
#include "def.h"

namespace msg{


struct resolv_task;


#define HintTCP 0
#define HintUDP 1
#define HintActive  0 // connect
#define HintPassive 1 // listen

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