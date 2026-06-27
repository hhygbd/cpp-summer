#include<iostream>
#include<cstring>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>

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

    //5、主循环 accept->recv->send->close
    while(true){
        struct sockaddr_in client_addr;
        socklen_t client_len=sizeof(client_addr);
        int client_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&client_len);
        if(client_fd==-1){
            std::cerr<<"accspt error"<<std::endl;
            continue;
        }

        std::cout<<"Client connected: "<<inet_ntoa(client_addr.sin_addr)<<std::endl;

        //处理客户端（循环读取直至关闭）
        while(true){
            char buffer[1024];
            memset(&buffer,0,sizeof(buffer));
            ssize_t recv_len=recv(client_fd,&buffer,sizeof(buffer)-1,0);
            if(recv_len==0)
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
            send(client_fd,buffer,recv_len,0);
        }
        close(client_fd);
    }
    close(listen_fd);


    return 0;
}