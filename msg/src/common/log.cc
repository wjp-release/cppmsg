#include "log.h"
namespace msg{namespace common{
namespace log{



static log_level lv = lv_debug;

void set_log_level(int x) {
    std::lock_guard<std::mutex> lk(mtx);
    lv = x;
}

void log_debug(const char* file, int line, const std::string& what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_debug<lv) return;
    va_list args;
    va_start(args, what);
    fprintf(stdout, "DEBUG, File %s Line %d: ", file, line);
    vfprintf(stdout, what, args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}

void log_err(const char* file, int line, const std::string& what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_err<lv) return;
    va_list args;
    va_start(args, what);
    fprintf(stdout, "ERR, File %s Line %d: ", file, line);
    vfprintf(stdout, what, args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}

void log_wtf(const char* file, int line, const std::string& what, ...) {
    std::lock_guard<std::mutex> lk(mtx);
    if(lv_wtf<lv) return;
    va_list args;
    va_start(args, what);
    log(lv_wtf, what, args);
    fprintf(stdout, "WTF, File %s Line %d: ", file, line);
    vfprintf(stdout, what, args);
    fprintf(stdout, "\n"); //auto indent
    va_end(args);
}



}
}}
