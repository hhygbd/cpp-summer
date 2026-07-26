#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include"EventLoop.h"
#include"TcpConnection.h"
#include<unordered_map>
#include<memory>

class TcpServer{
public:
    TcpServer(EventLoop* loop, int port);
    ~TcpServer();
    void start();

private:

    void handleAccept();

    void handleRead(int client_fd);
    void handleWrite(int client_fd);
    void handleError(int client_fd);

    void removeConnection(int client_fd);
    EventLoop* loop_;
    int listen_fd_;
    int port_;

    Channel* listen_channel_;


    std::unordered_map<int,std::shared_ptr<TcpConnection>> connections_;

    std::unordered_map<int,std::unique_ptr<Channel>> client_channels_;

};

#endif
