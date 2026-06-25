#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

#include <iostream>
#include <algorithm>

// 节点结构体
struct Node {
    int data;
    Node* next;
    Node(int val) : data(val), next(nullptr) {}
};

class LinkedList {
private:
    Node* head_;  // 头指针，指向第一个节点，空链表时为 nullptr

public:
    // 构造函数
    LinkedList();
    
    // 拷贝构造函数
    LinkedList(const LinkedList& other);

    // 拷贝赋值运算符
    LinkedList& operator=(const LinkedList& other);
    
    // 析构函数（释放所有节点）
    ~LinkedList();
    
    // 头插法
    void insertHead(int value);
    
    // 打印所有节点
    void print() const;
};

// 构造函数：将 head_ 初始化为 nullptr
LinkedList::LinkedList():head_(nullptr){
    // 你写

}

// 拷贝构造函数
LinkedList::LinkedList(const LinkedList& other){

    if(other.head_==nullptr)return;//原来的链表为空链表则退出退出拷贝构造

    this->head_=new Node(other.head_->data);

    //双指针
    Node* otherptr=other.head_;
    Node* thisptr=this->head_;

    while(otherptr->next!=nullptr){

        thisptr->next=new Node(otherptr->next->data);//构造节点
        //更新指针
        thisptr=thisptr->next;
        otherptr=otherptr->next;

    }

}

// 拷贝赋值运算符
LinkedList& LinkedList::operator=(const LinkedList& other){
    //首先防止自己赋值自己
    if(this==&other)return *this;

    LinkedList temp(other);//会调用内部的拷贝构造函数

    //交换内部指针
    std::swap(this->head_,temp.head_);
    
    return *this;//temp调用析构函数，带走旧内存
}

// 析构函数：遍历链表，逐个 delete 所有节点
LinkedList::~LinkedList() {
    // 你写
    while(head_!=nullptr){
        Node* temp=head_;
        head_=head_->next;
        delete temp;
    }
}

// 头插法：创建新节点，让新节点的 next 指向当前 head_，然后 head_ 指向新节点
void LinkedList::insertHead(int value) {
    // 你写
    Node* newone=new Node(value);
    newone->next=head_;
    head_=newone;
}

// 打印所有节点 (格式: 30 -> 20 -> 10 -> nullptr)
void LinkedList::print() const {
    const Node* current = head_; // 注意：因为函数是const，但遍历指针本身不影响
    while (current != nullptr) {
        std::cout << current->data;
        if (current->next != nullptr) {
            std::cout << " -> ";
        }
        current = current->next;
    }
    std::cout << " -> nullptr" << std::endl;
}

#endif