#include "stack.hpp"
#include "queue.hpp"
#include <iostream>
#include <string>

int main() {
    // ========== 测试 Stack ==========
    std::cout << "=== Stack Test ===" << std::endl;
    Stack<int> s;
    
    std::cout << "Is stack empty? " << (s.empty() ? "Yes" : "No") << std::endl;
    
    s.push(10);
    s.push(20);
    s.push(30);
    
    std::cout << "Stack size: " << s.size() << std::endl;
    std::cout << "Top element: " << s.top() << std::endl;   // 30
    
    s.pop();
    std::cout << "After pop, top: " << s.top() << std::endl; // 20
    std::cout << "Stack size: " << s.size() << std::endl;
    
    // 测试修改栈顶元素
    s.top() = 999;
    std::cout << "After s.top() = 999, top: " << s.top() << std::endl;
    
    // 继续出栈
    s.pop();
    s.pop();
    std::cout << "After popping all, is empty? " << (s.empty() ? "Yes" : "No") << std::endl;
    
    // ========== 测试 Queue ==========
    std::cout << "\n=== Queue Test ===" << std::endl;
    Queue<std::string> q;
    
    std::cout << "Is queue empty? " << (q.empty() ? "Yes" : "No") << std::endl;
    
    q.push("hello");
    q.push("world");
    q.push("!");
    
    std::cout << "Queue size: " << q.size() << std::endl;
    std::cout << "Front element: " << q.front() << std::endl; // hello
    
    q.pop();
    std::cout << "After pop, front: " << q.front() << std::endl; // world
    std::cout << "Queue size: " << q.size() << std::endl;
    
    // 测试修改队头元素
    q.front() = "hi";
    std::cout << "After q.front() = \"hi\", front: " << q.front() << std::endl;
    
    // 继续出队
    q.pop();
    q.pop();
    std::cout << "After popping all, is empty? " << (q.empty() ? "Yes" : "No") << std::endl;
    
    return 0;
}