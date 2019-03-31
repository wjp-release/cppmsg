#pragma once

namespace msg{namespace posix{

// settings
void     set_nonblock(int fd);
void     ignore_sigpipe(int fd);
// eventfd
int      efd_open();
void     efd_send(int efd);
void     efd_recv(int efd);
// signalfd
int      signalfd_open(int* signals, int n);
bool     signalfd_read(int signalfd, int* signo);
// timerfd
int      timerfd_open();





}}