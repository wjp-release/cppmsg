#include "log.h"

#include <cstdint> // uint64_t 
#include <cstddef>
#include <cstdio>
#include <mutex>

namespace msg{
namespace log{

static int lv = lv_debug;
static std::mutex mtx;

void set_log_level(int x) {
    std::lock_guard<std::mutex> lk(mtx);
    lv = x;
}

void log_debug(const char* file, int line, const std::string& what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_debug<lv) return;
    va_list args;
    va_start(args, what);
    fprintf(stdout, "<Debug> %s, line %d: ", file, line);
    vfprintf(stdout, what.c_str(), args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}

void log_err(const char* file, int line, const std::string& what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_err<lv) return;
    va_list args;
    va_start(args, what);
    fprintf(stdout, "<Error>, %s, line %d: ", file, line);
    vfprintf(stdout, what.c_str(), args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}

void log_wtf(const char* file, int line, const std::string& what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_wtf<lv) return;
    va_list args;
    va_start(args, what);
    fprintf(stdout, "<WTF>, %s, line %d: ", file, line);
    vfprintf(stdout, what.c_str(), args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}




}}
