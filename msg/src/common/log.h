#pragma once

#include "def.h"

namespace msg{namespace common{

namespace log{

enum log_level : int {
    debug       = 1,
    error       = 6,
    wtf         = 7,
};

void set_log_level(int loglv);
void log_debug(const char* file, int line, const std::string&, ...);
void log_err(const char* file, int line, const std::string&, ...);
void log_wtf(const char* file, int line, const std::string&, ...);

#define debug(...) log_debug(__FILE__, __LINE__, __VA_ARGS__)
#define err(...) log_err(__FILE__, __LINE__, __VA_ARGS__)
#define wtf(...) log_wtf(__FILE__, __LINE__, __VA_ARGS__)


}

    
}}