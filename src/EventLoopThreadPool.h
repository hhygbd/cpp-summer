#ifndef EVENT_LOOP_THREAD_POOL
#define EVENT_LOOP_THREAD_POOL

#include"EventLoop.h"
#include"EventLoopThread.h"
#include<vector>
#include<memory>

class EventLoopThreadPool {

public:

    EventLoopThreadPool(EventLoop* baseloop, int threadNum);
    ~EventLoopThreadPool();

    void start();
    EventLoop* getNextLoop();

private:

    EventLoop* baseloop_;
    int threadNum_;
    int next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;

};

#endif