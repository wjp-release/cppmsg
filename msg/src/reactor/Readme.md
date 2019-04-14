# Reactor Layer

The reactor layer implements a level-triggered epoll reactor, that could safely distribute event handling tasks to worker threads. 

It has following features:
1. It uses one-shot mode, which ensures that each event can only be processed by one worker thread. 
2. Destructors of event object can only be run by eventloop. The eventloop offers an interface to help release event objects for user threads. Therefore epoll_wait will never return a event of a fd that has just been closed by some random threads.
3. The eventloop is integrated with a timerfd-based cancellable timer, and a singalfd-based singal handling mechanism. 
