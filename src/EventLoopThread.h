#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include"EventLoop.h"
#include<thread>
#include<mutex>
#include<condition_variable>

class EventLoopThread {
public: 
    EventLoopThread();
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    void threadFunc();
    
    EventLoop* loop_;
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable cond_;
    bool exiting_;
};



#endif