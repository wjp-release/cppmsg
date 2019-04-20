#include "connection.h"

using namespace msg::transport;
namespace msg{namespace protocol{

// msghdr contains msglen, which allows creation of recv_msgbody_task
void recv_msghdr_task::on_success(int bytes){
    std::cout<<"hdr: msglen="<<hdr<<std::endl;
    c.conn->add_read(std::make_shared<recv_msgbody_task>(hdr, msg, shared_from_this()));
}

void recv_msghdr_task::on_failure(int err){
    if(err==msg::transport::peer_closed){
        //todo: replace iostream with a logger
        std::cerr<<"recv_msghdr_task peer closed"<<std::endl;
    }
}

// Now we have filled the msg, wake up user.
void recv_msgbody_task::on_success(int bytes){
    user_task->signal(); 
}

void recv_msgbody_task::on_failure(int err){
    if(err==peer_closed){
        std::cerr<<"recv_msgbody_task peer closed"<<std::endl;
    }
}

// Now we have filled the msg, wake up user.
void send_msg_task::on_success(int bytes){
    signal(); 
}

void send_msg_task::on_failure(int err){
    if(err==peer_closed){
        std::cerr<<"recv_msgbody_task peer closed"<<std::endl;
    }
}

}}