#include "linkedlist.hpp"
#include <iostream>
#include <string>

int main1() {
    // 测试1：int 类型的范围 for 循环
    LinkedList<int> list;
    list.insertHead(10);
    list.insertHead(20);
    list.insertHead(30);
    
    std::cout << "Int list (range-for): ";
    for (int x : list) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    // 测试2：string 类型的范围 for 循环
    LinkedList<std::string> strList;
    strList.insertHead("world");
    strList.insertHead("hello");
    
    std::cout << "String list (range-for): ";
    for (const std::string& s : strList) {
        std::cout << s << " ";
    }
    std::cout << std::endl;

    // 测试3：深拷贝是否仍正常工作（回归测试）
    LinkedList<int> list2 = list;
    list2.insertHead(999);
    
    std::cout << "Copy modified (range-for): ";
    for (int x : list2) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    std::cout << "Original unchanged (range-for): ";
    for (int x : list) {
        std::cout << x << " ";
    }
    std::cout << std::endl;

    return 0;
}