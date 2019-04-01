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

#include "fd.h"
#include "clock.h"

namespace msg{ namespace posix{

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
  int ret = timerfd_settime(timerfd, 0, &newValue, &oldValue); 
  if (ret) throw std::runtime_error("timerfd_settime failed");
}



}}