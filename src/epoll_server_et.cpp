#include<iostream>
#include<cstring>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<sys/epoll.h>
#include<errno.h>
#include<signal.h>
#include<fcntl.h>

#define MAX_EVENTS 10
#define PORT 8888

volatile sig_atomic_t running = 1;

void handle_sigint(int sig) {
    (void)sig;
    running = 0;
}

int set_nonblocking(int fd){
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

    //将listen_fd设为非阻塞
    if(set_nonblocking(listen_fd) == -1){
        std::cerr << "set_nonblocking listen_fd error" << std::endl;
        close(listen_fd);
        return 1;
    }

    //创建epoll实例
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        std::cerr << "epoll_create1 error" << std::endl;
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
    
    //将listen_fd加入epoll监听
    struct epoll_event ev;
    ev.events = EPOLLIN|EPOLLET;   //监听读事件
    ev.data.fd = listen_fd;
    
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,listen_fd,&ev) == -1){
        std::cout << "epoll_ctl and listen_fd error" << std::endl;
        close(listen_fd);
        close(epoll_fd);
        return 1; 
    }

    std::cout << "ET epoll server running on port: " << PORT << std::endl;

    //主事件循环

    epoll_event events[MAX_EVENTS];

    while(running){

        int nfds = epoll_wait(epoll_fd,events,MAX_EVENTS,-1);

        if(nfds == -1){
            if(errno == EINTR){
                continue;
            }
            std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
            break;
        }

        for(int i = 0; i < nfds; i++){
            //检查是否有新连接
            if(events[i].data.fd == listen_fd){
                //处理新的连接
                //ET模式下必须循环 accept，直到返回 EAGAIN
                while(running){
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);

                int client_fd = accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);
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
                if (set_nonblocking(client_fd) == -1) {
                    std::cerr << "set_nonblocking client_fd error" << std::endl;
                    close(client_fd);
                    continue;
                }

                //将client_fd加入epoll监听
                struct epoll_event client_ev;
                client_ev.events = EPOLLIN|EPOLLET;
                client_ev.data.fd = client_fd;

                if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,client_fd,&client_ev) == -1){
                    std::cerr << "epoll_ctl add client_fd error" << std::endl;
                    close(client_fd);
                    continue;
                    }
                }   
            }else{
                // 处理客户端数据（ET模式下必须循环 recv，直到返回 EAGAIN）
                int client_fd = events[i].data.fd;

                while(running){
                char buffer[1024];
                memset(buffer,0,sizeof(buffer));
                ssize_t recv_len = recv(client_fd,buffer,sizeof(buffer)-1,0);

                if(recv_len == 0){
                    //客户端主动断开（发送了FIN）
                    std::cout << "Client disconnect" << std::endl;
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,client_fd,nullptr);
                    close(client_fd);
                    break;//跳出recv循环
                }else if(recv_len < 0){
                    if(errno == EAGAIN || errno == EWOULDBLOCK){
                        // 所有数据都已读完，退出 recv 循环，回到 epoll_wait 等待下次通知
                        break;
                    }
                    // 真正的错误
                    std::cerr << "recv error: " << strerror(errno) << std::endl;
                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,client_fd,nullptr);
                    close(client_fd);
                    break;
                }else{
                    //接收到有效数据，执行echo（确保send完整发送）
                    ssize_t total_sent = 0;
                    while(total_sent < recv_len){
                        ssize_t sent = send(client_fd,buffer+total_sent,recv_len-total_sent,0);
                        if(sent < 0){
                            // 如果只是因为缓冲区满了（EAGAIN），在非阻塞下很少见，但为了严谨可以只 break 然后等待下一次 EPOLLOUT。
                            if(errno != EAGAIN && errno != EWOULDBLOCK){
                            std::cerr << "send error: " << strerror(errno) << ",close connection" << std::endl;
                            //（简单的 Echo），只要出错（特别是 ECONNRESET 或 EPIPE），直接关闭连接。
                            epoll_ctl(epoll_fd,EPOLL_CTL_DEL,client_fd,nullptr);
                            close(client_fd);   
                                }
                            break;
                            }
                        total_sent += sent;
                        }
                        //LT模式下这里读完一次就结束了，但ET模式下这里不能 break！
                    }
                }
            }
        }
    }
    //清理资源
    std::cout << "Shutting down..." << std::endl;
    close(listen_fd);
    close(epoll_fd);
 
    return 0;
}