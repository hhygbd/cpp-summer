#include"TcpServer.h"
#include"EventLoop.h"
#include<iostream>

int main(){
    EventLoop loop;
    TcpServer server(&loop, 8888);

    server.start();

    std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;

    loop.loop();

    return 0;
}