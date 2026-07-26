#include"Epoll.h"
#include"Channel.h"
#include<cerrno>
#include<unistd.h>
#include<iostream>
#include<cstring>
#include<stdexcept>
Epoll::Epoll():epoll_fd_(epoll_create1(0)),active_events_(64){
    if( epoll_fd_ < 0){
        throw std::runtime_error(std::string("epoll_create1 error: ") + strerror(errno));
        exit(1);
    }
}

Epoll::~Epoll(){
    close(epoll_fd_);
}

void Epoll::update(int operation, Channel* channel){
    struct epoll_event ev;
    memset(&ev,0,sizeof(ev));
    ev.events = channel->Get_events();
    ev.data.ptr = channel;

    if(epoll_ctl(epoll_fd_,operation,channel->Get_fd(),&ev) == -1){
        std::cerr << "epoll_ctl error: " << strerror(errno) << std::endl;
    }
}

void Epoll::updateChannel(Channel* channel){

    const int index = channel->index();

    if(channel->Get_events() == 0){

        if(index == 1){
            update(EPOLL_CTL_DEL, channel);
            channel->setIndex(-1);
        }
    }else{

        if(index == -1){
            update(EPOLL_CTL_ADD, channel);
            channel->setIndex(1);
        }else{
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void Epoll::removeChannel(Channel* channel){

    if(channel->index() == 1){
        update(EPOLL_CTL_DEL, channel);
        channel->setIndex(-1);
    }
}

std::vector<Channel*> Epoll::poll(int timeout_ms){
   
    int nfds = epoll_wait(epoll_fd_, active_events_.data(), static_cast<int>(active_events_.size()), timeout_ms);
    
    if (nfds == -1) {
        // 被信号中断
        if (errno == EINTR) {
            // 返回空列表
            return {};
        }
        // 其他真正的错误（如 epoll_fd 被意外关闭）
        std::cerr << "epoll_wait error: " << strerror(errno) << std::endl;
        return {};
    }   

    if (nfds == 0) {
        return {};
    }   

    std::vector<Channel*> ready_channels;
    ready_channels.reserve(nfds);  
    
    for (int i = 0; i < nfds; ++i) {
        
        Channel* ch = static_cast<Channel*>(active_events_[i].data.ptr);
       
        ch->setRevents(active_events_[i].events);
        
        ready_channels.push_back(ch);
    }
    
    return ready_channels;

}