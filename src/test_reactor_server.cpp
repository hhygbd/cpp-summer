#include"TcpServer.h"
#include"EventLoop.h"
#include<iostream>
#include <signal.h>

EventLoop* g_loop = nullptr;

void signalHandler(int sig) {
    if (sig == SIGINT && g_loop) {
        g_loop->quit();
    }
}

int main(int argc, char* argv[]) {
    signal(SIGINT, signalHandler);
    int threadNum = 4;
    if (argc > 1) {
        int val = std::atoi(argv[1]);
        if (val >= 0) {
            threadNum = val;
        } else {
            std::cerr << "Invalid thread number: " << argv[1] 
                      << ", using default 4" << std::endl;
        }
    }
    EventLoop loop;
    g_loop = &loop;
    TcpServer server(&loop, 8888, threadNum);
    server.start();
    loop.loop();
    std::cout << "Server stopped." << std::endl;
    return 0;
}