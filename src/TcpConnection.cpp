#include"TcpConnection.h"
#include<unistd.h>
#include<sys/socket.h>
#include<cstring>
#include<errno.h>
#include<iostream>

TcpConnection::TcpConnection(int fd) : fd_(fd){
}

TcpConnection::~TcpConnection(){
        close(fd_);
}

bool TcpConnection::handleRead(){

        char buffer[1024];
        memset(buffer,0,sizeof(buffer));

        while(true){           
            
            ssize_t recv_len = recv(fd_,buffer,sizeof(buffer)-1,0);

            if(recv_len > 0){
                input_buffer_ .append(buffer,recv_len);
            }else if(recv_len == 0){
                //客户端主动断开（发送了FIN）
                std::cout << "Client disconnect" << std::endl;
                return false;
            }else if(recv_len < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    // 所有数据都已读完，退出 recv 循环，回到 epoll_wait 等待下次通知
                    break;
                }
                // 真正的错误
                std::cerr << "recv error: " << strerror(errno) << std::endl;
                return false;
            }
        }
        return true;
}

bool TcpConnection::handleWrite(){
        
        ssize_t total_sent = 0;
        ssize_t total_len = output_buffer_.size();

        while(total_sent < total_len){
           ssize_t sent = send(fd_, output_buffer_.c_str() + total_sent, total_len - total_sent, 0);
           if(sent > 0){
                
                total_sent += sent;
           
           }else if(sent == -1){
               
            if(errno == EAGAIN || errno == EWOULDBLOCK){

                break;//直接 break，等待 epoll 下次触发 EPOLLOUT 即可
        
            }
           //其他真正的错误（如连接断了）
           std::cerr << "send error: " << strerror(errno) << ",close connection" << std::endl;
           return false;
           }
           
        }

        if(total_sent > 0){
            output_buffer_.erase(0, total_sent);
        }
        return true;
}

int TcpConnection::Get_fd() const{
    return fd_;
}

bool TcpConnection::hasOutputData() const{
    return !output_buffer_.empty();
}

std::string TcpConnection::getAndClearInputBuffer(){
    std::string data = std::move(input_buffer_);
    input_buffer_.clear();
    return data;
}

void TcpConnection::appendOutputBuffer(const std::string& data){
    output_buffer_.append(data);
}