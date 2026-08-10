#include"EventLoopThread.h"

EventLoopThread::EventLoopThread():loop_(nullptr),exiting_(false){}


EventLoopThread::~EventLoopThread(){
    exiting_ = true;
    if(loop_ != nullptr){   //如果 loop 还在运行，需要让它退出
        loop_->quit();      //EventLoop::quit() 内部已经处理了跨线程唤醒
    }

    if(thread_.joinable()){//等待子线程结束
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop(){
    thread_ = std::thread(&EventLoopThread::threadFunc,this);
    {

        std::unique_lock<std::mutex> lock(mutex_);
        cond_.wait(lock,[this]{ return loop_ != nullptr; });

    }

    return loop_;
}

void EventLoopThread::threadFunc(){
    // 1. 在子线程中构造 EventLoop（必须在子线程构造，因为会记录线程 ID）
    EventLoop loop;
    // 2. 将 EventLoop 地址赋给 loop_，并唤醒主线程
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = &loop;
        cond_.notify_one();
    }
    // 3. 进入事件循环（必须在锁外执行，否则死锁）
    loop.loop();
     // 4. 事件循环退出后，将 loop_ 置为 nullptr
    {
        std::unique_lock<std::mutex> lock(mutex_);
        loop_ = nullptr;
    }

}