#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include"Epoll.h"
#include<functional>
#include<vector>
#include<thread>
#include<mutex>
#include<sys/eventfd.h>
#include<atomic>

class EventLoop{
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    //跨线程任务接口
    void runInLoop(std::function<void()> cb);
    void queueInLoop(std::function<void()> cb);

    //获取当前线程ID，用于判断调用者是否在 EventLoop 线程中
    bool isInLoopThread () const { return threadId_ == std::this_thread::get_id(); }

    void updateChannel(Channel* channel);

    void removeChannel(Channel* channel);

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

private:
    
    void wakeup();
    void handleReadEventFd();
    void doPendingFunctors();

    Epoll epoll_;
    std::atomic<bool> quit_;

    int eventfd_;
    std::unique_ptr<Channel> eventfdchannel_;
    std::vector<std::function<void()>> pendingFunctors_;
    std::mutex mutex_;

    std::thread::id threadId_;

};


#endif