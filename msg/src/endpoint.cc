#include "endpoint.h"
#include "channel/connection.h"
namespace msg{

status endpoint::connect(const addr& a, int& newfd){
    int fd = socket(a.posix_family(), SOCK_STREAM | SOCK_CLOEXEC, 0);
    if(fd<0) return status::failure("create socket failed");
    addr_posix sp;
    int len=a.to_posix(&sp);
    if(::connect(fd, sp.sa(), len)!=0){
        return status::failure("connect failed");
    }
    try{
        connections[fd]=std::make_shared<message_connection>(fd);
        newfd=fd;
    }catch(...){
        return status::failure("create connection failed");
    }
    return status::success();
}

status endpoint::accept(const addr& a, int& newfd){
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
    if (listen(fd, 128) != 0) {
        return status::failure("listen failed");
    }
    newfd = accept4(fd, NULL, NULL, SOCK_CLOEXEC);
    if (newfd < 0) {
        return status::failure("accept4 failed");
    }
    return status::success();
}



    
}