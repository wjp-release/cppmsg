#include "endpoint.h"

namespace msg{

void endpoint::remove_connection(const connptr& c){
    auto it=std::find(connections.begin(),connections.end(), c);
    if(it!=connections.end()){
        connections.erase(it);
    }
}

void endpoint::create_message_connection(int fd){
    connections.push_back(std::make_shared<message_connection>(fd));
}

void endpoint::create_reliable_connection(int fd){
    connections.push_back(std::make_shared<reliable_connection>(fd));
}

void endpoint::create_very_reliable_connection(int fd){
    connections.push_back(std::make_shared<very_reliable_connection>(fd));
}
    
void endpoint::create_direct_connection(int fd){
    connections.push_back(std::make_shared<direct_connection>(fd));
}


}