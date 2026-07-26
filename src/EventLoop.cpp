#include"EventLoop.h"
#include"Channel.h"
#include<iostream>

EventLoop::EventLoop() : quit_(false) {
    //空实现
}

EventLoop::~EventLoop(){ 
    //空实现
}

void EventLoop::loop() {
    while(!quit_.load()){

        std::vector<Channel*> active_channels = epoll_.poll(-1);

        for(Channel* ch : active_channels){
            ch->handleEvent();
        }

    }
    //退出循环后，打印一条日志，方便调试
    std::cout << "EventLoop stopped." << std::endl;
}

void EventLoop::quit(){
    quit_.store(true);
}

void EventLoop::updateChannel(Channel* channel){
    epoll_.updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel){
    epoll_.removeChannel(channel);
}