#pragma once

#include "def.h"

namespace msg{namespace posix{

struct timespec parse_ms(int ms);
uint64_t        now();
uint64_t        now_depreciated();
void            sleep(int ms);
struct timespec timespec_elapsed(uint64_t start_time);
uint64_t        ms_elapsed(uint64_t start_time);
uint64_t        future(uint64_t ms_from_now);
    
}}