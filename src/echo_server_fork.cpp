#include<iostream>
#include<cstring>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<signal.h>

//
volatile sig_atomic_t running = 1;

void handle_sigint(int sig){
    (void)sig;
    running = 0;
}

int main(){

    //防止子进程变成僵尸进程
    signal(SIGCHLD,SIG_IGN);

    //socket
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd == -1){
        std::cerr << "socket error" << std::endl;
        return 1;
    }

    //setsockopt 端口复用
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
    addr.sin_port = htons(8888);

    if(bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr)) == -1){
        std::cerr << "bind error: " << strerror(errno) << std::endl;
        close(listen_fd);
        return 1;
    }

    //listen
    if(listen(listen_fd,10) == -1){
        std::cerr << "listen error..." << std::endl;
        close(listen_fd);
        return 1;
    }

    std::cout << "Fork server is running on port 8888..." << std::endl;

    //注册 SIGINT 信号
    struct sigaction sa;
    memset(&sa,0,sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT,&sa,nullptr);

    //主循环 accept->fork->子进程处理
    while(running){
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);
        if(client_fd < 0){
            if(errno != EINTR){
                std::cerr << "accept error: " << strerror(errno) << std::endl;
            }
            continue;
        }

        std::cout << "Client connect: " << inet_ntoa(client_addr.sin_addr) << std::endl;

        //fork主体
        pid_t pid = fork();

        if(pid == 0)
        {
            //在子进程内

            close(listen_fd);//不需要监听

            char buffer[1024];

            while(true){
                memset(buffer,0,sizeof(buffer));
                ssize_t recv_len = recv(client_fd,buffer,sizeof(buffer)-1,0);
                if(recv_len == 0){
                    std::cout << "Client disconnect" << std::endl;
                    break;
                }else if(recv_len < 0){
                    std::cerr << "recv error" << std::endl;
                    break;
                }

                ssize_t total_sent = 0;
                while(total_sent < recv_len){
                    ssize_t sent = send(client_fd,buffer+total_sent,recv_len-total_sent,0);
                    if(sent < 0){
                        std::cerr << "send error" << strerror(errno) << std::endl;
                        break;
                    }
                    total_sent += sent;
                }
                
            }
            close(client_fd);
            exit(0);
        }else if(pid > 0){
            //在父进程里 继续accept下一个连接
            close(client_fd);
        }else if(pid < 0){
            //创建子进程失败
            std::cerr << "fork error" << std::endl;
            close(client_fd);
        }

    }
    std::cout << "Shutting down gracefully..." << std::endl;

    close(listen_fd);

    return 0;
}