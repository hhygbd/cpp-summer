#include"TcpConnection.h"
#include<unistd.h>
#include<sys/socket.h>
#include<cstring>
#include<errno.h>
#include<iostream>

TcpConnection::TcpConnection(EventLoop* loop, int fd) : loop_(loop), fd_(fd), state_(kConnecting){

    //创建 Channel，接管该 fd 的事件管理
    channel_ = std::make_unique<Channel>(fd_);

    //将 Channel 的回调绑定到 TcpConnection 的成员函数
    channel_->setReadCallback([this](){ handleRead(); });
    channel_->setWriteCallback([this](){ handleWrite(); });
    channel_->setErrorCallback([this](){ handleError(); });       
}

TcpConnection::~TcpConnection(){
    //确保fd_被关闭
    if(fd_ >= 0){
        ::close(fd_);
        fd_ = -1;
    }
}

void TcpConnection::connectEstablished(){

    // 核心范式：确保在 loop_ 线程中执行注册操作
    // 使用 shared_from_this() 保证回调执行期间对象存活
    loop_->runInLoop(std::bind(&TcpConnection::connectEstablishedInLoop, shared_from_this()));
}

void TcpConnection::connectDestroyed(){

    // 核心范式：确保在 loop_ 线程中执行注销操作
    loop_->runInLoop(std::bind(&TcpConnection::connectDestroyedInLoop, shared_from_this()));
}

void TcpConnection::connectEstablishedInLoop(){

    SetState(kConnected);
    channel_->enableReading();
    loop_->updateChannel(channel_.get());

}

void TcpConnection::connectDestroyedInLoop(){

    if(state_.load() == kConnected){
        SetState(kDisconnected);
        channel_->disableAll();//关闭所有事件
        loop_->removeChannel(channel_.get());//从Epoll中移除
    }

}

void TcpConnection::send(const std::string& msg){

    if(state_.load() == kConnected){
        //确保在 loop_ 线程中执行发送
        loop_->runInLoop(std::bind(&TcpConnection::sendInLoop,shared_from_this(),msg));
    }
}

void TcpConnection::sendInLoop(const std::string& msg){
    if(state_.load() != kConnected) return;

    ssize_t nwrote = 0;
    ssize_t remaining = msg.size();

    if(output_buffer_.empty()){//如果输出缓冲区为空，尝试直接发送（减少一次 epoll 事件）

        nwrote = ::send(fd_, msg.data(), msg.size(), MSG_NOSIGNAL);

        if(nwrote >= 0){
            remaining = msg.size() - nwrote;
        }else{
            nwrote = 0;
            if(errno != EAGAIN && errno != EWOULDBLOCK){
                std::cerr << "TcpConnection::sendInLoop send error: " << strerror(errno) << std::endl;
                return;
            }
        }
    }

    if(remaining > 0){//如果没发完，放入输出缓冲区，并监听 EPOLLOUT
        output_buffer_.append(msg.data() + nwrote, remaining);
        if(!channel_->isWriting()){
            channel_->enableWriting();
            loop_->updateChannel(channel_.get());
        }
    }
}

void TcpConnection::forceClose(){
    if(state_.load() == kConnected || state_.load() == kDisconnecting){
        SetState(kDisconnecting);
        loop_->runInLoop(std::bind(&TcpConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TcpConnection::forceCloseInLoop(){
    if (state_.load() == kConnected || state_.load() == kDisconnecting) {
        handleClose();
    }
}

void TcpConnection::handleRead(){

        char buffer[65536];
        memset(buffer,0,sizeof(buffer));

        while(true){           
            
            ssize_t recv_len = ::recv(fd_,buffer,sizeof(buffer)-1,0);

            if(recv_len > 0){
                input_buffer_ .append(buffer,recv_len);
            }else if(recv_len == 0){
                //客户端主动断开（发送了FIN）
                handleClose();
                return;
            }else if(recv_len < 0){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    // 所有数据都已读完，退出 recv 循环，回到 epoll_wait 等待下次通知
                    break;
                }
                // 真正的错误
                std::cerr << "TcpConnection::handleRead recv error: " << strerror(errno) << std::endl;
                handleClose();
                return;
            }
        }
        
        if(!input_buffer_.empty()){
            std::string msg = std::move(input_buffer_);
            input_buffer_.clear();
            send(msg);//直接回显

        }
}

void TcpConnection::handleWrite(){

    if (output_buffer_.empty()) {
        if (channel_->isWriting()) {
            channel_->disableWriting();
            loop_->updateChannel(channel_.get());
        }
        return;
    }
        
    ssize_t total_sent = 0;
    ssize_t total_len = output_buffer_.size();

    while(total_sent < total_len){

       ssize_t sent = ::send(fd_, output_buffer_.c_str() + total_sent, total_len - total_sent, MSG_NOSIGNAL);

       if(sent > 0){
            
            total_sent += sent;
       
       }else if(sent == -1){
           
            if(errno == EAGAIN || errno == EWOULDBLOCK){
            break;//直接 break，等待 epoll 下次触发 EPOLLOUT 即可
    
            }
            //其他真正的错误（如连接断了）
            std::cerr << "TcpConnection::handleWrite send error: " << strerror(errno) << std::endl;
            handleClose();
            return;

       }else{
            // sent == 0，理论上不会发生，但防御处理
            std::cerr << "TcpConnection::handleWrite send returned 0" << std::endl;
            handleClose();
            return;
       }
    }

    if(total_sent > 0){
        output_buffer_.erase(0, total_sent);
    }

    // 如果数据全部发完，关闭 EPOLLOUT
    if (output_buffer_.empty()) {
        if (channel_->isWriting()) {
            channel_->disableWriting();
            loop_->updateChannel(channel_.get());
        }
        // 如果状态是 kDisconnecting，且数据已发完，可以关闭连接
        if (state_.load() == kDisconnecting) {
            handleClose();
        }
    }
}

void TcpConnection::handleError(){
    std::cerr << "TcpConnection::handleError fd = " << fd_ << std::endl;
    handleClose();
}

void TcpConnection::handleClose() {
    // 防止重复关闭
    if (state_.load() == kDisconnected) return;

    SetState(kDisconnected);
    channel_->disableAll();
    loop_->removeChannel(channel_.get());

}

