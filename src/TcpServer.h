#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include"EventLoop.h"
#include"TcpConnection.h"
#include"EventLoopThreadPool.h"
#include<unordered_map>
#include<memory>

class TcpServer{
public:
    //threadNum = 0 表示不创建子线程，退化为单 Reactor
    TcpServer(EventLoop* loop, int port, int threadNum = 0);
    ~TcpServer();

    void start();

private:

    void handleAccept();
    void removeConnection(const TcpConnectionPtr& conn);
    void removeConnectionInLoop(const TcpConnectionPtr& conn);

    EventLoop* loop_;
    int listen_fd_;
    int port_;
    std::unique_ptr<Channel> listen_channel_;

    //管理所有活跃的连接
    std::unordered_map<int,TcpConnectionPtr> connections_;

    //子线程池
    std::unique_ptr<EventLoopThreadPool> threadPool_;

};

#endif
