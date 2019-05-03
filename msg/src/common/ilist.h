#pragma once

namespace msg{

//intrusive list node base
template<class T>
class inode{
public:
    virtual ~inode(){} //enable down cast
    void insert_prev(inode<T>* node){
        node->next=this;
        node->prev=prev;
        prev->next=node;
        this->prev=node;
    }
    void insert_next(inode<T>* node){
        node->next=next;
        node->prev=this;
        next->prev=node;
        this->next=node;
    }
    void unlink(){
        prev->next=next;
        next->prev=prev;
    }
    void swap(inode<T>* node){
        node->prev->next=this;
        node->next->prev=this;
        this->prev->next=node;
        this->next->prev=node;
        std::swap(this->next, node->next);
        std::swap(this->prev, node->prev);
    }
    bool is_linked()const{
        if(next==0||prev==0)return false;
        return next->prev==this && prev->next==this;
    }
    T* get(){
        return dynamic_cast<T*>(this);
    }
    inode* next = 0;
    inode* prev = 0;
};

//intrusive list
template <class T>
class ilist{
public:
    ilist(){
        head.next=&head;
        head.prev=&head;
    }
    void push_front(T*t){
        head.insert_next(t);
    }
    void push_back(T*t){
        head.insert_prev(t);
    }
    void pop_front(){
        if(!empty()) head.next->unlink();
    }
    void pop_back(){
        if(!empty()) head.prev->unlink();
    }
    T* front(){
        return static_cast<T*>(head.next);
    }
    T* back(){
        return static_cast<T*>(head.next);
    }
    inode<T>* begin(){
        return head.next;
    }
    inode<T>* rbegin(){
        return head.prev;
    }
    inode<T>* end(){
        return &head;
    }
    bool empty(){
        return head.next==&head;
    }
private:
    inode<T> head;
};


}