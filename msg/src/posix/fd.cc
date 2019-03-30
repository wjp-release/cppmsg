#include "epollfd.h"
#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/eventfd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/event.h>
#include <sys/socket.h>

namespace msg{ namespace posix{

void set_nonblock(int fd){
    fcntl(fd, F_SETFD, FD_CLOEXEC);
	fcntl(fd, F_SETFL, O_NONBLOCK);
}

void ignore_sigpipe(int fd){
    int one = 1;
	setsockopt(fd, SOL_SOCKET, SO_NOSIGPIPE, &one, sizeof(one)); 
}

int efd_open(){
    int fd=eventfd(0, EFD_CLOEXEC);
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    fcntl(fd, F_SETFL, O_NONBLOCK);
    return fd;
}

void efd_send(int efd){
    uint64_t one=1;
    write(efd,&one,sizeof(one));
}

void efd_recv(int efd){
    uint64_t duh;
    read(efd, &duh, sizeof(duh));
}

static void block_all_signals(){
    sigset_t all;
    sigfillset(&all);
    sigprocmask(SIG_SETMASK,&all,NULL);
}

int signalfd_open(int* signals, int n){
    block_all_signals();
    sigset_t tmp;
    sigemptyset(&tmp);
    for(int i=0; i <n ; i++){
        sigaddset(&tmp, signals[i]);
    } 
    return signalfd(-1, &tmp, 0);
}

bool signalfd_read(int signalfd, int* signo){
    struct signalfd_siginfo info;
    ssize_t nr = read(signalfd, &info, sizeof(info));
    if (nr != sizeof(info)) {
        return false;
    }
    signo=info.ssi_signo;
    return true;
}


}}