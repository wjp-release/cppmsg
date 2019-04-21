#pragma once

namespace msg{namespace common{

//intrusive list node
class inode{
public:
    
protected:
    inode* next = 0;
    inode* prev = 0;
};

//intrusive list
template <class T>
class ilist{

};


}}