# cppmsg

cppmsg is a message library written in C++11.

1. Safe concurrent event processing based on epoll's one-shot mode.
2. Zero application level copy based on readv & writev.
3. Efficient timer based on timerfd.
4. Repliable message protocol(user-level resend, ack and heartbeats) based on nonblocking tcp. 

## build
cd msg/scripts
./clean.sh
./build_min_release_ninja (or build_min_release_cmake)




