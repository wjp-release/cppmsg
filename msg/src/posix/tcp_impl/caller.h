#pragma once

#include "def.h"
#include <mutex>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "conn.h"

namespace msg{ namespace posix{ namespace tcpimpl{

class connecting_conn{
    struct sockaddr_storage addr;
	size_t      addr_len;
};

class caller{
public:


private:
    std::vector<connecting_conn> connecting_conns;
	bool        closed;
	bool        nodelay;
	bool        keepalive;
	std::mutex  mtx;
};

}}} 