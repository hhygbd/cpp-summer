#ifndef QUEUE_HPP
#define QUEUE_HPP

#include "linkedlist.hpp"

template<typename T>
class Queue {
private:
    LinkedList<T> list_;   // 底层容器

public:
    // 入队：在链表尾部插入
    void push(const T& value) {
        list_.pushBack(value);
    }

    // 出队：删除头部节点
    void pop() {
        list_.popFront();
    }

    // 获取队头元素（可修改版本）
    T& front() {
        return *list_.begin();   // 用迭代器返回头部数据引用
    }

    // 获取队头元素（只读版本）
    const T& front() const {
        return list_.front();    // 用 front() 返回 const 引用
    }

    // 判断队列是否为空
    bool empty() const {
        return list_.empty();
    }

    // 获取队列的大小
    size_t size() const {
        return list_.size();
    }
};

#endif