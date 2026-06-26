#ifndef STACK_HPP
#define STACK_HPP

#include"linkedlist.hpp"

template<typename T>
class Stack{
private:
    LinkedList<T> list_;//底层容器组合方法
public:
    //将元素压入栈顶
    void push(const T& value){
        this->list_.insertHead(value);
    }

    //删除头节点
    void pop(){
        this->list_.popFront();
    }

    //返回栈顶数据(不可修改版本)
    const T& top() const {
        return this->list_.front();
    }
    //返回栈顶数据（可修改版本）
    T& top() {
        return *this->list_.begin();
    }


    //查看栈是否为空
    bool empty() const{
        return this->list_.empty();
    }

    //当前栈中元素个数
    size_t size() const{
        return this->list_.size();
    }

};


#endif 