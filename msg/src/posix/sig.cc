#include "sig.h"
#include "fd.h"
#include "event.h"
#include <stdlib.h>
#include <iostream>
#include <exception>
#include <stdexcept>
#include <signal.h>
#include "event.h"

namespace msg{namespace posix{

static int sigs[] = {SIGIO,SIGHUP,SIGTERM,SIGINT,SIGQUIT,SIGALRM};

static int signalfd=-1;

static int create_signalfd(){
    signalfd=signalfd_open(sigs, sizeof(sigs));
    return signalfd;
}

event* create_signalfd_event(){
    if(signalfd==-1) create_signalfd();
    if(signalfd==-1) throw std::runtime_error("fail to create signalfd!");
    return new event(signalfd, SIG_EV, []{
        int signo;
        signalfd_read(signalfd, &signo);
        std::cerr<<"got signal "<<signo<<std::endl; 
        exit(-1);
    });
}

}}