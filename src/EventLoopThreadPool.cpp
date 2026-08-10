#include"EventLoopThreadPool.h"


EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseloop, int threadNum)
  : baseloop_(baseloop),
    threadNum_(threadNum),
    next_(0){
        if(threadNum < 0){
            threadNum_ = 0;
        }
    }

EventLoopThreadPool::~EventLoopThreadPool(){

}

void EventLoopThreadPool::start(){
    for(int i = 0; i < threadNum_; i++){
        auto t = std::make_unique<EventLoopThread>();
        EventLoop* loop = t->startLoop();
        threads_.push_back(std::move(t));
        loops_.push_back(loop);
    }
}

EventLoop* EventLoopThreadPool::getNextLoop(){
    if(threadNum_ == 0 || loops_.empty()){
        return baseloop_;
    }

    EventLoop* loop = loops_[next_];
    next_ = (next_ + 1) % loops_.size();
    return loop;
}