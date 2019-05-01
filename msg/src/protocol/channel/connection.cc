#include "connection.h"

namespace msg{
    
message_connection::recv_msghdr_task::recv_msghdr_task(message_connection& c, message& msg) : oneiov_read_task(reinterpret_cast<void*>(&hdr), 8), c(c), msg(msg){}

// msghdr contains msglen, which allows creation of recv_msgbody_task
void message_connection::recv_msghdr_task::on_success(int bytes){
    logdebug("%d bytes read, hdr contains msglen, which is %d", bytes, (int)hdr);
    if(hdr>message::MaxSize){
        signal(); //obviously illegal, discard it 
        return;
    }
    c.c->add_read(std::make_shared<recv_msgbody_task>(hdr, msg, shared_from_this()));
}

void message_connection::recv_msghdr_task::on_recoverable_failure(){
}

message_connection::recv_msgbody_task::recv_msgbody_task(int size, message& msg, std::shared_ptr<blockable> user_task): 
    oneiov_read_task(msg.alloc(size), size),
    user_task(user_task){}

// Now we have filled the msg, wake up user.
void message_connection::recv_msgbody_task::on_success(int bytes){
    logdebug("%d bytes read", bytes);
    user_task->signal(); 
}

void message_connection::recv_msgbody_task::on_recoverable_failure(){
}

message_connection::send_msg_task::send_msg_task(const message& msg) : vector_write_task(msg.nr_chunks()+1, msg.size()){
    msg.append_iov(iovs); 
}

// Now we have filled the msg, wake up user.
void message_connection::send_msg_task::on_success(int bytes){
    logerr("%d bytes written", bytes);
    signal(); 
}

void message_connection::send_msg_task::on_recoverable_failure(){
}

connection::connection(int fd):c(std::make_unique<conn>(fd)){}

void message_connection::sendmsg(const message& msg){
    auto task=std::make_shared<send_msg_task>(msg);
    c->add_write(task);
    task->wait();
}

void message_connection::recvmsg(message& msg){
    auto task=std::make_shared<recv_msghdr_task>(*this, msg);
    c->add_read(task);
    task->wait();
}



}