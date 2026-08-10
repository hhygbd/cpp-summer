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

int main() {
    signal(SIGINT, signalHandler);
    EventLoop loop;
    g_loop = &loop;
    TcpServer server(&loop, 8888, 4);
    server.start();
    loop.loop();
    std::cout << "Server stopped." << std::endl;
    return 0;
}