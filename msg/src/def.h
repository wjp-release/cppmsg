#pragma once

// frequently used headers
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

// compilation options for reliablility test
#define ENABLE_CHAOS_MONKEY_TEST // change successful results to random failures randomly
#define ENABLE_ADVERSARY_TEST // change successful results to a certain failure deterministicly

// msg:: definitions
namespace msg{

// 1024*4KB = 4MB 
const static int arena_pool_size = 256;
const static int arena_chunk_size = 4096;
const static int large_message_size = 0; 

// The send/recv retry backoff time can increases to at most 5 seconds
const static int backoff_max=5120; //ms
// Async timeout is implemented by checking backoff value
const static int backoff_subjectively_down=2000; //ms
const static int backoff_base=22; // always let backoff+= backoff_base to prevent backoff from being 0

class addr;
using resolv_cb=std::function<void(addr)>;
//async_cb(int x) x>0 on success, represents bytes, x<0 on failure
using async_cb=std::function<void(int)>;
const static int AsyncTimeout = -1; // async timeout
const static int WillRetryLater = -2; // recoverable failure retry
//event_cb(int flag) takes evflag
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

