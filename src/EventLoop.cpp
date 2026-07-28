#include"EventLoop.h"
#include"Channel.h"
#include<iostream>
#include<unistd.h>
#include<cstring>

EventLoop::EventLoop() : quit_(false), eventfd_(-1), threadId_(std::this_thread::get_id()){
    eventfd_ = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(eventfd_ == -1){
        std::cerr << "eventfd creation failed: " << strerror(errno) << std::endl;
        exit(1);
    }

    eventfdchannel_ = std::make_unique<Channel>(eventfd_);
    eventfdchannel_->setReadCallback([this]() { handleReadEventFd(); });
    eventfdchannel_->enableReading();
    epoll_.updateChannel(eventfdchannel_.get());
}

EventLoop::~EventLoop(){ 
    if(eventfd_ != -1){
        close(eventfd_);
        eventfd_ = -1;
    }
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
    wakeup();
}

void EventLoop::runInLoop(std::function<void()> cb){
    if(isInLoopThread()){
        cb();
    }else{
        queueInLoop(cb);
    }
}

void EventLoop::queueInLoop(std::function<void()> cb){
    {
        std::unique_lock<std::mutex> lock(mutex_);
        pendingFunctors_.push_back(std::move(cb));
    }
    wakeup();
}

void EventLoop::wakeup(){

    // 向 eventfd_ 写入一个 8 字节的整数
    // 这会让内核触发 eventfd_ 的 EPOLLIN 事件
    // 从而让正在 epoll_wait 阻塞的主线程立即返回
    uint64_t one = 1;
    ssize_t n = write(eventfd_,&one,sizeof(one));
    if(n != sizeof(one)){
        std::cerr << "EventLoop::wakeup() writes " << n << " bytes instead of 8" << std::endl;
    }
}

void EventLoop::handleReadEventFd(){
    uint64_t one = 1;
    ssize_t n = read(eventfd_,&one,sizeof(one));
    if(n == sizeof(one)){
        doPendingFunctors();
    }else{
        std::cerr << "EventLoop::handleReadEventFd() reads " << n << " bytes instead of 8" << std::endl;
    }
}

void EventLoop::doPendingFunctors(){

    std::vector<std::function<void()>> functors;
    {
        std::unique_lock<std::mutex> lock(mutex_);
        functors.swap(pendingFunctors_);
    }

    for(const auto& cb : functors){
        cb();
    }
}

void EventLoop::updateChannel(Channel* channel){
    epoll_.updateChannel(channel);
}

void EventLoop::removeChannel(Channel* channel){
    epoll_.removeChannel(channel);
}