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

//If the type of what is const std::string&, it could possibly attempt to alloc heap memory for a temporary string object, which is subideal.
void log_debug(const char* file, int line, const char* what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_debug<lv) return;
    va_list args;
    va_start(args, what);
    fprintf(stdout, "<Debug> %s, line %d: ", file, line);
    vfprintf(stdout, what, args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}

void log_err(const char* file, int line, const char* what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_err<lv) return;
    va_list args;
    va_start(args, what);
    fprintf(stdout, "<Error>, %s, line %d: ", file, line);
    vfprintf(stdout, what, args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}

void log_wtf(const char* file, int line, const char* what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_wtf<lv) return;
    va_list args;
    va_start(args, what);
    fprintf(stdout, "<WTF>, %s, line %d: ", file, line);
    vfprintf(stdout, what, args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}




}}
