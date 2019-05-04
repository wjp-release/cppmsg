#pragma once

#include <functional> // function
#include <cstdint> // uint64_t 
#include <cstddef>
#include <cstdarg>
#include <cstdio>
#include <time.h> // timespec
#include <exception>
#include <stdexcept>
#include <iostream>
#include <memory>
#include <mutex>
#include <list>
#include <vector>
#include <sys/uio.h> //struct iovec
#include <string.h>
#include <algorithm>
#include <cassert>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <cstring>
#include <unistd.h>
#include "common/log.h"
#include "common/status.h"
#include "common/concurrentq.h"
#include "common/ilist.h"

namespace msg{

class addr;
using resolv_cb=std::function<void(addr)>;
using async_cb=std::function<void(int)>;
using event_cb=std::function<void(int)>;
using please_cb=std::function<void(void)>;
using timer_cb=std::function<void(void)>;
using task_cb=std::function<void(void)>;
using failure_cb=std::function<void(int, const std::vector<iovec>&)>;
using transferred_cb=std::function<void(int, const std::vector<iovec>&)>;

template<class T>
using cq=moodycamel::ConcurrentQueue<T>; // todo: use token feature to further speed up producer-consumer 

}

#define WJP_DEBUG

