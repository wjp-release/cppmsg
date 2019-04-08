#pragma once

#include "def.h"

namespace msg{namespace common{

// settings
void     set_nonblock(int fd);
// eventfd
int      efd_open();
void     efd_send(int efd);
void     efd_recv(int efd);
// signalfd
int      signalfd_open(int* signals, int n);
bool     signalfd_read(int signalfd, int* signo);
// timerfd
int      timerfd_open();
uint64_t timerfd_read(int timerfd);
void     timerfd_reset(int timerfd, uint64_t expiration_time);



}}