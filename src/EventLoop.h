#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include"Epoll.h"
#include<atomic>

class EventLoop{
public:
    EventLoop();
    ~EventLoop();

    void loop();

    void quit();

    void updateChannel(Channel* channel);

    void removeChannel(Channel* channel);

    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

private:
    
    Epoll epoll_;
    std::atomic<bool> quit_;

};


#endif