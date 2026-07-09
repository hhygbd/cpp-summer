#include<iostream>
#include<cstring>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<signal.h>
#include<pthread.h>

//信号安全全局运行标志
volatile sig_atomic_t running = 1;

//SIGINT中断信号处理函数
void handle_sigint(int sig){
    (void)sig;
    running = 0;
}

//线程函数：处理单个客户端
void* client_handle(void* arg){
    int client_fd = *(int*)arg;
    delete (int*)arg;//释放动态分配的内存

    char buffer[1024];
    while(true){
        memset(buffer,0,sizeof(buffer));
        ssize_t recv_len = recv(client_fd,buffer,sizeof(buffer),0);
        if(recv_len == 0){
            std::cout << "Client disconnect" << std::endl;
            break;
        }else if(recv_len < 0){
            std::cerr << "recv error" << std::endl;
            break;
        }

        //send
        ssize_t total_sent = 0;
        while(total_sent < recv_len){
            ssize_t sent = send(client_fd,buffer+total_sent,recv_len-total_sent,0);
            if(sent < 0){
                std::cerr << "send error: " << strerror(errno) << std::endl;
                break;
            }
            total_sent += sent;
        }

    }
    close(client_fd);
    return nullptr;

}

int main(){
    //1、socket
    int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd == -1){
        std::cerr << "socket error" << std::endl;
        return 1;
    }

    //2、setsockopt 端口复用
    int opt = 1;
    if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) == -1){
        std::cerr << "setsockopt error" << std::endl;
        close(listen_fd);
        return 1;
    }

    //3、bind
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(8888);

    if(bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr)) == -1){
        std::cerr << "bind error" << strerror(errno) << std::endl;
        close(listen_fd);
        return 1;
    }

    //4、listen
    if(listen(listen_fd,10) == -1){
        std::cerr << "listen error" << std::endl;
        close(listen_fd);
        return -1;
    }

    std::cout << "Pthread server is running on port 8888..." << std::endl;

    //5、注册信号
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);

    //6、主循环 accept->pthread_create
    while(running){
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);

        if(client_fd == -1){
            if(errno != EINTR){
                std::cerr << "accept error: " << strerror(errno) << std::endl;
            }
            continue;
        }

        std::cout << "Client connected: " << inet_ntoa(client_addr.sin_addr) << std::endl;

        //动态分配client_fd,确保线程能拿到自己的fd
        int* pclient_fd = new int(client_fd);

        pthread_t tid;
        if(pthread_create(&tid,nullptr,client_handle,pclient_fd) != 0){
            std::cerr << "pthread_creata error" << std::endl;
            delete pclient_fd;
            close(client_fd);
            continue;
        }

        pthread_detach(tid);//线程退出时自动回收资源

    }

    std::cout << "Shutting down gracefully..." << std::endl;
    close(listen_fd);
    return 0;

}