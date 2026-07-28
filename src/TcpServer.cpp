#include"TcpServer.h"
#include"Channel.h"
#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/socket.h>
#include<cstring>
#include<arpa/inet.h>

TcpServer::TcpServer(EventLoop* loop, int port)
    :loop_(loop),
    port_(port),
    listen_fd_(-1),
    listen_channel_(nullptr){
    //socket
    listen_fd_ = socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd_ == -1){
        std::cerr << "socket error" << std::endl;
        exit(1);
    }

    //setsockopt
    int opt = 1;
    if(setsockopt(listen_fd_,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) == -1){
        std::cerr << "setsockopt error" << std::endl;
        close(listen_fd_);
        exit(1);
    }

    //bind
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port_);

    if(bind(listen_fd_,(struct sockaddr*)&addr,sizeof(addr)) == -1){
        std::cerr << "bind error: " << strerror(errno) << std::endl;
        close(listen_fd_);
        exit(1); 
    }

    //listen
    if(listen(listen_fd_,10) == -1){
        std::cerr << "listen error" << std::endl;
        close(listen_fd_);
        exit(1);
    }

    //将listen_fd_设为非阻塞
    //获取文件描述符当前的所有标志
    int flags = fcntl(listen_fd_,F_GETFL,0);
    if(flags == -1){
        std::cerr << "fcntl F_GETFL error" << strerror(errno) << std::endl;
        close(listen_fd_);
        exit(1);
    }
    if(fcntl(listen_fd_,F_SETFL,flags|O_NONBLOCK) == -1){
        std::cerr << "fcntl F_SETFL error" << strerror(errno) << std::endl;
        close(listen_fd_);
        exit(1);
    }

    listen_channel_ = new Channel(listen_fd_);

    listen_channel_->setReadCallback([this](){
        this->handleAccept();
    });

    std::cout << "TcpServer initialized on port " << port_ << std::endl;

    threadPool_ = std::make_unique<ThreadPool>(4);


}

void TcpServer::start() {
    // listen_channel_：关注读事件（新连接到达）
    listen_channel_->enableReading();

    // 2. 把 Channel 注册到 EventLoop 中
    //    这让 Epoll 开始真正监听 listen_fd_
    loop_->updateChannel(listen_channel_);

    std::cout << "TcpServer started. Listening on port " << port_ << std::endl;
}

void TcpServer::handleAccept(){
    while(true){
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = accept(listen_fd_,(struct sockaddr*)&client_addr,&client_len);
        if(client_fd == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;// 所有等待的连接都已处理完毕，退出循环
            }
            //真正的错误
            std::cerr << "accept error: " << strerror(errno) << std::endl;
            break;
        }

        std::cout << "New connection from: " << inet_ntoa(client_addr.sin_addr) << std::endl;
        // 将 client_fd 设为非阻塞
        int flags = fcntl(client_fd,F_GETFL,0);
        if(flags == -1){
            std::cerr << "fcntl F_GETFL client error" << strerror(errno) << std::endl;
            close(client_fd);
            continue;
        }
        if(fcntl(client_fd,F_SETFL,flags|O_NONBLOCK) == -1){
            std::cerr << "fcntl F_SETFL client error" << strerror(errno) << std::endl;
            close(client_fd);
            continue;
        }

        //创建 TcpConnection（管理业务逻辑和缓冲区）
        auto conn = std::make_shared<TcpConnection>(client_fd);
        connections_[client_fd] = conn;

        //创建 Channel（管理事件分发）
        auto ch = std::make_unique<Channel>(client_fd);
        client_channels_[client_fd] = std::move(ch);

        //绑定回调
        client_channels_[client_fd]->setReadCallback([this, client_fd]() {
            handleRead(client_fd);
        });
        client_channels_[client_fd]->setWriteCallback([this, client_fd]() {
            handleWrite(client_fd);
        });
        client_channels_[client_fd]->setErrorCallback([this, client_fd]() {
            handleError(client_fd);
        });

        //开始监听读事件
        client_channels_[client_fd]->enableReading();
        loop_->updateChannel(client_channels_[client_fd].get());

    }

}

void TcpServer::handleRead(int client_fd) {
    auto it = connections_.find(client_fd);
    if (it == connections_.end()) {
        std::cerr << "handleRead: connection not found for fd " << client_fd << std::endl;
        return;
    }

    auto& conn = it->second;

    bool alive = conn->handleRead();
    if (!alive) {
        removeConnection(client_fd);
        return;
    }

    std::string msg = conn->getAndClearInputBuffer();
    if(msg.empty()){
        return;
    }

    threadPool_->addTask([this,client_fd,msg]{

        std::this_thread::sleep_for(std::chrono::seconds(3));
        std::string response = msg;

        loop_->runInLoop([this,client_fd,response]{
            handleWriteInLoop(client_fd,response);
        });
    });

}

void TcpServer::handleWrite(int client_fd) {
    auto it = connections_.find(client_fd);
    if (it == connections_.end()) {
        std::cerr << "handleWrite: connection not found for fd " << client_fd << std::endl;
        return;
    }

    it->second->handleWrite();
}

void TcpServer::handleError(int client_fd) {
    auto it = connections_.find(client_fd);
    if (it == connections_.end()) {
        std::cerr << "handleError: connection not found for fd " << client_fd << std::endl;
        return;
    }

    std::cerr << "Error occurred on fd " << client_fd << std::endl;
    removeConnection(client_fd);
}

void TcpServer::removeConnection(int client_fd) {
    // 1. 从 EventLoop 中移除 Channel
    auto ch_it = client_channels_.find(client_fd);
    if (ch_it != client_channels_.end()) {
        loop_->removeChannel(ch_it->second.get());
        client_channels_.erase(ch_it);
    }

    // 2. 从 connections_ 中移除 TcpConnection（自动释放资源）
    auto conn_it = connections_.find(client_fd);
    if (conn_it != connections_.end()) {
        connections_.erase(conn_it);
    }

    std::cout << "Connection closed: fd " << client_fd << std::endl;
}

TcpServer::~TcpServer() {
    // 移除 listen_channel 并释放内存
    if (listen_channel_) {
        loop_->removeChannel(listen_channel_);
        delete listen_channel_;
        listen_channel_ = nullptr;
    }
    // 关闭监听套接字
    if (listen_fd_ >= 0) {
        close(listen_fd_);
    }
}

void TcpServer::handleWriteInLoop(int client_fd, const std::string& msg){
    auto conn_it = connections_.find(client_fd);
    if(conn_it == connections_.end()){
        return;
    }

    auto& conn = conn_it->second;

    conn->appendOutputBuffer(msg);

    bool alive = conn->handleWrite();
    if(!alive){
        removeConnection(client_fd);
        return;
    }

    if(conn->hasOutputData()){
        auto ch_it = client_channels_.find(client_fd);
        if(ch_it != client_channels_.end()){
            ch_it->second->enableWriting();
            loop_->updateChannel(ch_it->second.get());
        }
    }else{
        auto ch_it = client_channels_.find(client_fd);
        if(ch_it != client_channels_.end()){
            ch_it->second->disableWriting();
            loop_->updateChannel(ch_it->second.get());
        }
    }
}
