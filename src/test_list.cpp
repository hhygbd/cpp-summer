#include "linkedlist.hpp"
#include <iostream>

int main() {
    LinkedList list;
    list.insertHead(10);
    list.insertHead(20);
    list.insertHead(30);
    
    std::cout << "Original: ";
    list.print(); // 预期: 30 -> 20 -> 10

    // 测试拷贝构造
    LinkedList list2 = list; 
    list2.insertHead(999);
    
    std::cout << "Original after copy: ";
    list.print();   // 预期不变: 30 -> 20 -> 10
    std::cout << "Copy with new head: ";
    list2.print();  // 预期: 999 -> 30 -> 20 -> 10

    return 0;
}
