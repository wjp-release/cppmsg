#pragma once

#include "def.h"

namespace msg{

class taskpool {
public:
    ~taskpool();
    taskpool() = delete;
    taskpool(taskpool &&) = default;
    static taskpool& instance();
    void execute(const task_cb& task);
private:
    struct meta;
    explicit taskpool(size_t nr_thread);
    std::shared_ptr<meta> meta_;
};


}