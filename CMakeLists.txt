cmake_minimum_required(VERSION 3.10)
project(CppMultiReactor)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -Wall")
find_package(Threads REQUIRED)
# 多 Reactor 服务器
add_executable(test_reactor_server
    src/test_reactor_server.cpp
    src/TcpServer.cpp
    src/TcpConnection.cpp
    src/EventLoopThreadPool.cpp
    src/EventLoopThread.cpp
    src/EventLoop.cpp
    src/Epoll.cpp
    src/Channel.cpp
)
target_link_libraries(test_reactor_server PRIVATE Threads::Threads)
# 压测客户端
add_executable(load_test_client
    tools/load_test_client.cpp
)
target_link_libraries(load_test_client PRIVATE Threads::Threads)
