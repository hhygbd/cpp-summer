#include "linkedlist.hpp"
#include <iostream>
#include <string>

int main() {
    // 测试 int
    LinkedList<int> list;
    list.insertHead(10);
    list.insertHead(20);
    list.insertHead(30);
    std::cout << "Int list: ";
    list.print();

    // 测试拷贝构造（int）
    LinkedList<int> list2 = list;
    list2.insertHead(999);
    std::cout << "Int list (copy modified): ";
    list2.print();
    std::cout << "Original int list unchanged: ";
    list.print();

    // 测试 std::string
    LinkedList<std::string> strList;
    strList.insertHead("world");
    strList.insertHead("hello");
    std::cout << "String list: ";
    strList.print();

    // 测试拷贝构造（string）
    LinkedList<std::string> strList2 = strList;
    strList2.insertHead("hi");
    std::cout << "String list (copy modified): ";
    strList2.print();
    std::cout << "Original string list unchanged: ";
    strList.print();

    return 0;
}