#include "connection.h"

using namespace msg::transport;
namespace msg{namespace protocol{
    
recv_msghdr_task::recv_msghdr_task(connection& c, message& msg) : transport::oneiov_read_task(reinterpret_cast<void*>(&hdr), 8), c(c), msg(msg){}

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

recv_msgbody_task::recv_msgbody_task(int size, message& msg, std::shared_ptr<common::blockable> user_task): 
    transport::oneiov_read_task(msg.alloc(size), size),
    user_task(user_task){}

// Now we have filled the msg, wake up user.
void recv_msgbody_task::on_success(int bytes){
    user_task->signal(); 
}

void recv_msgbody_task::on_failure(int err){
    if(err==peer_closed){
        std::cerr<<"recv_msgbody_task peer closed"<<std::endl;
    }
}

send_msg_task::send_msg_task(const message& msg) : transport::vector_write_task(msg.nr_chunks()+1, msg.size()){
    msg.append_iov(iovs); 
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

connection::connection(endpoint& ep, int fd):owner(ep), conn(std::make_unique<msg::transport::conn>(fd)){

}


}}