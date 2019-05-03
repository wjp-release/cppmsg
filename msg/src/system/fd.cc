#include <sys/timerfd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/eventfd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <string.h>
#include <exception>
#include <stdexcept>
#include <iostream>
#include <sys/types.h>
#include <errno.h>
#include "fd.h"
#include "common/clock.h"

namespace msg{ 

// SO_REUSEADDR allows the server to bind to an address which is in a TIME_WAIT state.
// Without SO_REUSEADDR, the bind() call in the restarted program's new instance will fail if there were connections open to the previous instance when you killed it. Those connections will hold the TCP port in the TIME_WAIT state for 30-120 seconds.
void set_sockfd_reuse_addr(int fd){
    int duh = 1;
    (void) setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &duh, sizeof(duh));
}

// The result of an asynchronous connect can only be extracted via getsockopt.
int get_sockfd_err(int fd){
    int result;
    socklen_t resultlen = sizeof(result);
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &result, &resultlen) < 0) result = errno;
    return result;
}

void set_nonblock(int fd){
    fcntl(fd, F_SETFD, FD_CLOEXEC);
	fcntl(fd, F_SETFL, O_NONBLOCK);
}

int efd_open(){
    int fd=eventfd(0, EFD_CLOEXEC);
    fcntl(fd, F_SETFD, FD_CLOEXEC);
    fcntl(fd, F_SETFL, O_NONBLOCK);
    return fd;
}

uint64_t efd_send(int efd){
    uint64_t one=1;
    if(write(efd,&one,sizeof(one))<0) throw std::runtime_error("efd write failed");
    return one;
}

uint64_t efd_recv(int efd){
    uint64_t duh;
    if(read(efd, &duh, sizeof(duh))<0) throw std::runtime_error("efd read failed");
    return duh;
}

void block_all_signals(){
    sigset_t all;
    sigfillset(&all);
    sigprocmask(SIG_SETMASK,&all,NULL);
}

// fixme: signalfd doesn't work as expected if eventloop run in a background thread. 
int signalfd_open(int* signals, int n){
    // block_all_signals(); 
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
    if (nr != sizeof(info)) return false;
    *signo=info.ssi_signo;
    return true;
}

int timerfd_open(){
  return timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
}

// read over timerfd returns an uint64_t containing the number of expirations that have occurred
uint64_t timerfd_read(int timerfd)
{
  uint64_t duh; 
  ssize_t n = ::read(timerfd, &duh, sizeof duh);
  if (n != sizeof duh) throw std::runtime_error("timer fd read bytes n!=8");
  //std::cout<<"timerfd read "<<n<<"bytes\n";
  return duh; // normally it should be 1
}

//itimerspec{it_interval, it_value}  interval, initial expiration 
void timerfd_reset(int timerfd, uint64_t expiration_time)
{
  struct itimerspec newValue;
  struct itimerspec oldValue;
  memset(&newValue, 0, sizeof newValue);
  memset(&oldValue, 0, sizeof oldValue);
  newValue.it_value = parse_ms(expiration_time-now());
  //std::cout<<"timerfd reset to "<<(expiration_time-now())<<" later"<<std::endl;
  int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue); 
  if (ret) throw std::runtime_error("timerfd_settime failed");
}



}