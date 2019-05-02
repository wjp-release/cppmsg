#pragma once

#include "common/ilist.h"
#include <mutex>

namespace msg{

class connection;

class global{
public:
    static global& instance(){
        static global g;
        return g;
    }
    void add_connection(connection* c){

    }
    void remove_connection(connection* c){

    }
private:
    ilist<connection> connections;
    std::mutex  connections_mtx;

};
    
}