#include "endpoint.h"
#include "channel/basic_connection.h"

namespace msg{

status endpoint::connect(const addr& a, int& newfd){
    int fd = socket(a.posix_family(), SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(fd<0) return status::failure("create socket failed");
    addr_posix sp;
    int len=a.to_posix(&sp);
    if(::connect(fd, sp.sa(), len)!=0){
        return status::failure("connect failed");
    }
    return status::success();
}

status endpoint::listen(const addr& a, int& listenfd){
    int fd = socket(a.posix_family(), SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(fd<0){
        return status::failure("create socket failed");
    }
    set_sockfd_reuse_addr(fd);
    addr_posix sp;
    int len=a.to_posix(&sp);
    if(bind(fd, sp.sa(), len)<0){
        return status::failure("bind failed");
    }
    if (::listen(fd, 128) != 0) {
        return status::failure("listen failed");
    }
    listenfd=fd;
    return status::success();
}

status endpoint::accept(int listenfd, int&newfd){
    // accept4(listenfd, 0, 0, SOCK_CLOEXEC) is better 
    // but unavailable on many platforms
    newfd = ::accept(listenfd, NULL, NULL);
    if (newfd < 0) {
        return status::failure("accept4 failed");
    }
    return status::success();
}

    
}