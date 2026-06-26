#ifndef LINKEDLIST_HPP
#define LINKEDLIST_HPP

#include <iostream>
#include <algorithm>

// 节点结构体
template<typename T>
struct Node {
    T data;
    Node<T>* next;//因为现在的 Node 全名是 Node<T>
    Node(const T& val) : data(val), next(nullptr) {}
};

template<typename T>
class LinkedList {
private:

    Node<T>* head_;  // 头指针，指向第一个节点，空链表时为 nullptr

public:

    class Iterator{
    private:

        Node<T>* ptr_;

    public:
        //构造函数
        Iterator(Node<T>* p=nullptr):ptr_(p){};

        //解引用 返回当前节点的数据引用
        T& operator*() const {//加const，函数内部不更改数据
            return this->ptr_->data;
        }

        //前置自增的运算符重载
        Iterator& operator++(){
            ptr_=ptr_->next;
            return *this;
        }

        //后置自增的运算符重载
        Iterator operator++(int){//int为占位符，区分前后自增
            Iterator temp = *this;
            this->ptr_=this->ptr_->next;
            return temp;
        }

        //不等于运算符重载,当且仅当它们指向不同的节点
        bool operator!=(const Iterator &other) const {
            
            return this->ptr_!=other.ptr_;

        }
    };

    //新增的begin(),end()
    Iterator begin(){ return Iterator(head_); }
    Iterator end(){ return Iterator(nullptr); }
    // 构造函数
    LinkedList();
    
    // 拷贝构造函数
    LinkedList(const LinkedList<T>& other);

    // 拷贝赋值运算符
    LinkedList<T>& operator=(const LinkedList<T>& other);
    
    // 析构函数（释放所有节点）
    ~LinkedList();
    
    // 头插法
    void insertHead(const T& value);
    
    //尾插法
    void pushBack(const T& value);

    //获取头节点数据
    const T& front() const{
        return this->head_->data;
    }

    //删除头节点
    void popFront(){
        if(this->head_ == nullptr) return;//空链表直接删除
        //非空链表
        Node<T>* temp=this->head_;
        head_=head_->next;
        delete(temp);
    }

    //判断链表是否为空
    bool empty() const {
        return this->head_ == nullptr;
    }

    //获取链表大小（遍历计数）
    size_t size() const {
        size_t count=0;
        Node<T>* current=this->head_;
        while(current!=nullptr){
            count++;
            current=current->next;
        }
        return count;
    }

    // 打印所有节点
    void print() const;
};

template<typename T>
// 构造函数：将 head_ 初始化为 nullptr
LinkedList<T>::LinkedList():head_(nullptr){
    

}

template<typename T>
// 拷贝构造函数
LinkedList<T>::LinkedList(const LinkedList<T>& other){

    if(other.head_==nullptr)return;//原来的链表为空链表则退出退出拷贝构造

    this->head_=new Node<T>(other.head_->data);

    //双指针
    Node<T>* otherptr=other.head_;
    Node<T>* thisptr=this->head_;

    while(otherptr->next!=nullptr){

        thisptr->next=new Node<T>(otherptr->next->data);//构造节点
        //更新指针
        thisptr=thisptr->next;
        otherptr=otherptr->next;

    }

}

template<typename T>
// 拷贝赋值运算符
LinkedList<T>& LinkedList<T>::operator=(const LinkedList<T>& other){
    //首先防止自己赋值自己
    if(this==&other)return *this;

    LinkedList<T> temp(other);//会调用内部的拷贝构造函数

    //交换内部指针
    std::swap(this->head_,temp.head_);
    
    return *this;//temp调用析构函数，带走旧内存
}

template<typename T>
// 析构函数：遍历链表，逐个 delete 所有节点
LinkedList<T>::~LinkedList() {
    // 你写
    while(head_!=nullptr){
        Node<T>* temp=head_;
        head_=head_->next;
        delete temp;
    }
}

template<typename T>
// 头插法：创建新节点，让新节点的 next 指向当前 head_，然后 head_ 指向新节点
void LinkedList<T>::insertHead(const T& value) {
    // 你写
    Node<T>* newone=new Node<T>(value);
    newone->next=head_;
    head_=newone;
}

template<typename T>
//尾插法
void LinkedList<T>::pushBack(const T& value){
        if(head_==nullptr){//如果链表为空，则直接插入
            head_=new Node<T>(value);
            return;
        }
        //否则就遍历到头指针为空
        Node<T>* current=head_;
        while(current->next != nullptr){
            current=current->next;
        }
        current->next=new Node<T>(value);
    }

template<typename T>
// 打印所有节点 (格式: 30 -> 20 -> 10 -> nullptr)
void LinkedList<T>::print() const {
    const Node<T>* current = head_; // 注意：因为函数是const，但遍历指针本身不影响
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