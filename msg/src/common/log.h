#pragma once
#include <cstdarg>
#include <string>

namespace msg{
namespace log{

enum log_level : int {
    lv_debug    = 1, //log everything
    lv_err      = 6, //log faults
    lv_wtf      = 7, //log critical faults
    lv_nolog    = 20 //log nothing
};

void set_log_level(int loglv);
void log_debug(const char* file, int line, const char*, ...);
void log_err(const char* file, int line, const char*, ...);
void log_wtf(const char* file, int line, const char*, ...);

}   
}

// macro is the only way to retrieve __FILE__ & __LINE__
#define logdebug(...) msg::log::log_debug(__FILE__, __LINE__, __VA_ARGS__)
#define logerr(...) msg::log::log_err(__FILE__, __LINE__, __VA_ARGS__)
#define logwtf(...) msg::log::log_wtf(__FILE__, __LINE__, __VA_ARGS__)
