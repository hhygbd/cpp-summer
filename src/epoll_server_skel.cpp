#include<iostream>
#include<cstring>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<signal.h>
#include<errno.h>
#include<fcntl.h>

#define MAX_EVENTS 10
#define PORT 8888

volatile sig_atomic_t running = 1;

void handle_sigint(int sig){
    (void)sig;
    running = 0;
}

int set_nonblocking(int fd){
    //第一步，先获取文件描述符当前的所有标志位
    int flags = fcntl(fd,F_GETFL,0);
    if(flags == -1){
        std::cerr << "fcntl F_GETFL error:" << strerror(errno) << std::endl;
        return -1;
    }
    //第二步：在原有标志位的基础上，添加 O_NONBLOCK(非阻塞)标志
    if(fcntl(fd,F_SETFL,flags|O_NONBLOCK) == -1){
        std::cerr << "fcntl F_SETFL error:" << strerror(errno) << std::endl;
        return -1;
    }
    return 0;
}

int main(){

    //socket
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd == -1){
        std::cerr << "socket error" << std::endl;
        return 1;
    }

    //setsockopt
    int opt = 1;
    if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) == -1){
        std::cerr << "setsockopt error" << std::endl;
        close(listen_fd);
        return 1;
    }

    //bind
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);

    if(bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr)) == -1){
        std::cerr << "bind error: " << strerror(errno) << std::endl;
        close(listen_fd);
        return 1; 
    }

    //listen
    if(listen(listen_fd,10) == -1){
        std::cerr << "listen error" << std::endl;
        close(listen_fd);
        return 1;
    }

    //将listen_fd设置为非阻塞
    if(set_nonblocking(listen_fd) == -1){
        std::cerr << "set_nonblocking error" << std::endl;
        close(listen_fd);
        return -1;
    }

    //创建epoll实例
    int epoll_fd = epoll_create1(0);//0为默认管理，系统已经能自动管理内存了
    if(epoll_fd == -1){
        std::cerr << "epoll_create error" << std::endl;
        close(listen_fd);
        return 1;
    }

    //注册SIGINT信号
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,&sa,nullptr);

    std::cout << "Epoll server running on port " << PORT << std::endl;

    //将listen_fd注册到epoll实例中，监听读事件
    struct epoll_event ev;
    ev.events = EPOLLIN; // 监听“可读”事件
    ev.data.fd = listen_fd;

    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd,&ev) == -1){
        std::cerr << "epoll_ctl and listen_fd error" << std::endl;
        close(listen_fd);
        close(epoll_fd);
        return 1;
    }


    //主事件循环
    struct epoll_event events[MAX_EVENTS];

    while(running){
        //阻塞等待epoll事件发生
        int nfds = epoll_wait(epoll_fd,events,MAX_EVENTS,-1);

        //处理epoll_wait返回值
        if(nfds == -1){
            //被信号（如 Ctrl+C）中断，继续循环
            if(errno == EINTR){
                continue;
            }
            std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
            break;
        }
        //遍历所有发生的事件
        for(int i = 0; i < nfds; i++){
            //判断listen_fd上是不是有新的连接
            if(events[i].data.fd == listen_fd){
                //有新连接到达，调用accept处理
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int client_fd = accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);

                if(client_fd == -1){
                    // 非阻塞模式下，如果没有连接，errno 会是 EAGAIN 或 EWOULDBLOCK
                    if(errno == EAGAIN || errno == EWOULDBLOCK){
                        continue;
                    }
                    std::cerr << "accept error" << strerror(errno) << std::endl;
                    continue;

                }
                
                //打印客户端IP地址
                std::cout << "New connection from: " << inet_ntoa(client_addr.sin_addr) << std::endl;

                close(client_fd);
            }
        }

    }

    //清理资源
    std::cout << "Shutting down..." << std::endl;
    close(listen_fd);
    close(epoll_fd);


    return 0;
}