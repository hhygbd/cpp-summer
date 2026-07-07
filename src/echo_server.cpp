#include<iostream>
#include<cstring>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<signal.h>

//sig_atomic_t 保证“读”和“写”是原子操作
//volatile 禁止编译器优化
volatile sig_atomic_t running = 1;

void handle_sigint(int sig)
{
    (void)sig;
    running = 0;
}
int main(){
    //1、创建socket

    int listen_fd = socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd == -1){
        std::cerr << "socket error" << std::endl;
        return 1;
    }

    //2、设置端口复用
    int opt=1;
    if(setsockopt(listen_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt))==-1){
        std::cerr << "setsockopt err" << std::endl;
        close(listen_fd);
        return 1;
    }

    //3、绑定地址和端口
    struct sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=INADDR_ANY;
    addr.sin_port=htons(8888);

    if(bind(listen_fd,(struct sockaddr*)&addr,sizeof(addr))==-1){
        std::cerr << "bind error:" << strerror(errno) << std::endl;
        close(listen_fd);
        return 1;
    }

    //4、开始监听
    if(listen(listen_fd,10)==-1){
        std::cerr<<"listen error"<<std::endl;
        close(listen_fd);
        return 1;
    }
    std::cout<<"Server is running on port 8888...."<<std::endl;

    //信号注册
    //按照我的写的handle_sigint来执行,不遵循默认执行
    //之前的步骤执行错误直接推出，不需要注册信号
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;   // 关键：不设置 SA_RESTART，禁止系统调用自动重启
    sigaction(SIGINT, &sa, nullptr);

    //5、主循环 accept->recv->send->close  
    //running(运行):1,running(终止):0；
    while(running){
        struct sockaddr_in client_addr;
        socklen_t client_len=sizeof(client_addr);
        int client_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);
        if(client_fd==-1){
            if(errno != EINTR){
                std::cerr<<"accept error: "<< strerror(errno) <<std::endl;
            }
            
            continue;
        }

        std::cout<<"Client connected: "<<inet_ntoa(client_addr.sin_addr)<<std::endl;

        //处理客户端（循环读取直至关闭）
        while(true){
            char buffer[1024];
            memset(&buffer,0,sizeof(buffer));
            ssize_t recv_len=recv(client_fd,&buffer,sizeof(buffer)-1,0);//recv()接受数据
            if(recv_len==0)//正常关闭
            {
                //客户端主动关闭
                 std::cout<<"Client disconnected: "<<std::endl;
                 break;
            }
            else if(recv_len<0)
            {
                // 出错了（比如连接重置）
                std::cerr<<"recv error"<<std::endl;
                break;

            }
            //将收到的数据原样发回（Echo）
            //修复send发送数据不完全可能出现的bug
            //send不能保证一次把所有数据发完
            ssize_t total_sent = 0;//记录已经发送可多少字节的数据
            while(total_sent < recv_len){
                //send返回已成功发送的字节数
                ssize_t sent = send(client_fd,buffer+total_sent,recv_len-total_sent,0);//buf+tol,指针偏移运算
                if(sent < 0){
                    //错误，跳出循环
                    std::cerr << "send error" << std::endl;
                    break;
                }
                total_sent += sent;
            }
            
        }
        close(client_fd);
    }

    std::cout << "Shutting down gracefully..." << std::endl;

    close(listen_fd);

    return 0;
}