#pragma once

#include "def.h"

namespace msg{namespace common{

// settings
void     set_nonblock(int fd);
// eventfd
int      efd_open();
uint64_t efd_send(int efd);
uint64_t efd_recv(int efd);
// signalfd
int      signalfd_open(int* signals, int n);
bool     signalfd_read(int signalfd, int* signo);
// timerfd
int      timerfd_open();
uint64_t timerfd_read(int timerfd);
void     timerfd_reset(int timerfd, uint64_t expiration_time);

int      get_sockfd_err(int fd);
void     set_sockfd_reuse_addr(int fd);

}}