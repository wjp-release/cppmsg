#pragma once

// A collection of test/sample helper functions

#define server_uds_path "/tmp/server.uds"
#define client_uds_path "/tmp/client.uds"

#include <string.h>
#include <assert.h>
#include <cstddef>
#include <cstdint>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string>
#include <string.h>
#include <sys/signalfd.h>
#include <sys/epoll.h>
#include <signal.h>
#include <unistd.h>

namespace msg{ namespace sample{

static inline void panic(const char* what){
    fprintf(stderr,"%s: %s\n", what, strerror(errno));
    exit(-1);
}

// For the sake of simplicity we use unix domain socket datagram mode to test the reactor.

static inline int ipc_sock(const char* path){
    int sock_fd=socket(AF_UNIX, SOCK_DGRAM, 0);
    if (sock_fd == -1) panic("socket");
    return sock_fd;
}

static inline void ipc_send(int fd, const char* what, size_t len){
    if(write(fd, what, len) == -1) panic("write");
}

static inline void ipc_send(int fd, const char* what){
    ipc_send(fd, what, strlen(what));
}

static inline void ipc_send(int fd, const std::string& what){
    ipc_send(fd, what.c_str());
}

// return a sockfd that's connected to path
static inline int ipc_connect(const char* path) {
    int sock_fd=ipc_sock(path);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    if(connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr))==-1) panic("connect");
    return sock_fd;
}

// return a sockfd that's binded to path
static inline int ipc_bind(const char* path) {
    int sock_fd=ipc_sock(path);
    unlink(path);
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path)-1);
    if(::bind(sock_fd, (struct sockaddr*)&addr, sizeof(addr))) panic("bind");
    return sock_fd;
}




}}
