#pragma once

#include <functional> // function
#include <cstdint> // uint64_t 
#include <time.h> // timespec
#include <exception>
#include <stdexcept>
#include <iostream>
#include <memory>
#include "concurrentq.h"

using event_cb=std::function<void(int)>;
using please_cb=std::function<void(void)>;
using timer_cb=std::function<void(void)>;
using task_cb=std::function<void(void)>;
using failure_cb=std::function<void(int)>;
using transferred_cb=std::function<void(void)>;

template<class T>
using cq=moodycamel::ConcurrentQueue<T>; // todo: use token feature to further speed up producer-consumer 

