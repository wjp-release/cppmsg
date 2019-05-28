#include "clock.h"
#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <math.h>

namespace msg{

struct timespec parse_ms(int ms){
    struct timespec t;
    t.tv_sec = ms / 1000;
    t.tv_nsec = (ms % 1000) * 1000000;
    return t;
}

uint64_t now(){
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return 1000*spec.tv_sec+ round(spec.tv_nsec / 1.0e6);
}

struct timespec now_spec(){
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    return spec;
}

std::pair<uint32_t,uint32_t> ns_elapsed(timespec& start){
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    uint32_t sdiff=spec.tv_sec-start.tv_sec;
    uint32_t nsdiff=spec.tv_nsec-start.tv_nsec;
    return std::make_pair(sdiff,nsdiff);
}

// In the current version of POSIX, gettimeofday is marked obsolete. 
uint64_t now_depreciated(){
    struct timeval tv;
    gettimeofday (&tv, NULL);
    return (int64_t) (tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

void sleep(int ms){
    struct timespec t;
    t.tv_sec = ms / 1000;
    t.tv_nsec = (ms % 1000) * 1000000;
    nanosleep (&t, NULL);
}

struct timespec timespec_elapsed(uint64_t start_time){
    return parse_ms(now()-start_time);
}

uint64_t ms_elapsed(uint64_t start_time){
    return now()-start_time;
}

uint64_t future(uint64_t ms_from_now){
    return ms_from_now+now();
}



}