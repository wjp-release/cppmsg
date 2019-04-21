#include "endpoint.h"

namespace msg{namespace protocol{

void endpoint::remove_connection(const connptr& c){
    auto it=std::find(connections.begin(),connections.end(), c);
    if(it!=connections.end()){
        connections.erase(it);
    }
}

void endpoint::create_connection(int fd){
    connections.push_back(std::make_shared<connection>(fd));
}



}}