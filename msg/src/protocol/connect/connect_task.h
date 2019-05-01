#pragma once

#include "def.h"
#include "system/addr.h"
#include "system/fd.h"

namespace msg{
class connector;

struct connect_task{
    connect_task()noexcept{}
    virtual ~connect_task(){}
    status          try_connect(connector* c) noexcept;
    virtual status  on_asnyc_success() noexcept=0;
    virtual status  on_failure() noexcept=0;
    status          handle_async_connect_result() noexcept;
    addr            a;
    int             newfd;
};

}