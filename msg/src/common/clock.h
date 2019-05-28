#pragma once

#include "def.h"

// clock.h: time related functions

namespace msg{

// nanosec measurement
struct timespec now_spec();
std::pair<uint32_t,uint32_t> ns_elapsed(timespec& start);


struct timespec parse_ms(int ms);
uint64_t        now();
uint64_t        now_depreciated();
void            sleep(int ms);
struct timespec timespec_elapsed(uint64_t start_time);
uint64_t        ms_elapsed(uint64_t start_time);
uint64_t        future(uint64_t ms_from_now);
    
}