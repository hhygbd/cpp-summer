#include"TcpServer.h"
#include<iostream>
#include<unistd.h>
#include<fcntl.h>
#include<errno.h>
#include<sys/socket.h>
#include<cstring>
#include<arpa/inet.h>

//设置非阻塞

static int setNonBlocking(int fd){
    //获取文件描述符当前的所有标志
    int flags = fcntl(fd,F_GETFL,0);
    if(flags == -1){
        std::cerr << "fcntl F_GETFL error" << strerror(errno) << std::endl;
        return -1;
    }
    if(fcntl(fd,F_SETFL,flags|O_NONBLOCK) == -1){
        std::cerr << "fcntl F_SETFL error" << strerror(errno) << std::endl;
        return -1;
    }

    return 0;
}

TcpServer::TcpServer(EventLoop* loop, int port, int threadNum)
    :loop_(loop),
    port_(port),
    listen_fd_(-1){
    //socket
    listen_fd_ = socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd_ == -1){
        std::cerr << "Tcpserver: socket error" << std::endl;
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
    if (setNonBlocking(listen_fd_) == -1) {
        std::cerr << "TcpServer: fcntl nonblocking error" << std::endl;
        close(listen_fd_);
        exit(1);
    }

    //创建listen_channel
    listen_channel_ = std::make_unique<Channel>(listen_fd_);
    listen_channel_->setReadCallback([this](){handleAccept();});

    //初始化线程
    threadPool_ = std::make_unique<EventLoopThreadPool>(loop_, threadNum);

    std::cout << "TcpServer initialized on port " << port_ << " with " << threadNum << " sub Reactors" << std::endl;
}

TcpServer::~TcpServer() {
    // 移除 listen_channel 并释放内存
    for(auto it : connections_){
        it.second->forceClose();
    }
    connections_.clear();
    
    // 关闭监听套接字
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }
}

void TcpServer::start() {

    //启动线程池
    threadPool_->start();

    // 将 listen_channel_ 注册到主 EventLoop
    listen_channel_->enableReading();
    loop_->updateChannel(listen_channel_.get());

    std::cout << "TcpServer started. Listening on port " << port_ << std::endl;
}

void TcpServer::handleAccept(){
    while(true){
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        int client_fd = ::accept(listen_fd_,(struct sockaddr*)&client_addr,&client_len);

        if(client_fd == -1){
            if(errno == EAGAIN || errno == EWOULDBLOCK){
                break;// 所有等待的连接都已处理完毕，退出循环
            }
            //真正的错误
            std::cerr << "TcpServer::handleAccept accept error: " << strerror(errno) << std::endl;
            break;
        }

        // 打印连接信息
        std::cout << "New connection from: " << inet_ntoa(client_addr.sin_addr) << ", fd: " << client_fd << std::endl;

        // 设置 client_fd 为非阻塞
        if (setNonBlocking(client_fd) == -1) {
            std::cerr << "TcpServer::handleAccept setNonBlocking error" << std::endl;
            close(client_fd);
            continue;
        }
        
        //分配子Reactor
        EventLoop* subLoop = threadPool_->getNextLoop();


        //创建 TcpConnection
        TcpConnectionPtr conn = std::make_shared<TcpConnection>(subLoop, client_fd);
        
        // 设置关闭回调（由 TcpConnection 在 handleClose 中触发）
        conn->setCloseCallback([this](const TcpConnectionPtr& c){
            removeConnection(c);
        });

        //存入 connections_ map（主线程操作，安全）
        connections_[client_fd] = conn;

        //跨线程注册 Channle (让子线程自己执行 epoll_ctl ADD)
        conn->connectEstablished();

    }

}

void TcpServer::removeConnection(const TcpConnectionPtr& conn) {
    // 这个函数由子线程的 TcpConnection::handleClose 触发
    // 必须回到主线程修改 connections_
    loop_->runInLoop(std::bind(&TcpServer::removeConnectionInLoop, this, conn));

}

void TcpServer::removeConnectionInLoop(const TcpConnectionPtr& conn){
    int fd = conn->fd();

    size_t erased = connections_.erase(fd);
    if(erased == 0){
        // 连接已被移除，忽略重复调用
        return;
    }

      std::cout << "Connection closed and removed: fd " << fd << std::endl;

      EventLoop* subLoop = conn->getLoop();
      subLoop->runInLoop(std::bind(&TcpConnection::connectDestroyed, conn));

}


