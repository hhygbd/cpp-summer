#include "EventLoopThread.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    EventLoopThread thread;

    EventLoop* loop = thread.startLoop();
    std::cout << "Main thread: got sub-loop pointer" << std::endl;

    loop->runInLoop([]() {
        std::cout << "Sub-thread: Hello from event loop!" << std::endl;
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));

    std::cout << "Main thread: done." << std::endl;
    return 0;
}