#ifndef EPOLL_H
#define EPOLL_H
#include<sys/epoll.h>
#include<vector>

class Channel;// 前置声明，避免循环包含

class Epoll{
public:
    Epoll();
    ~Epoll();
    // 将 Channel 注册到 epoll，或更新其监听事件
    void updateChannel(Channel* channel);
    // 从 epoll 中彻底移除 Channel
    void removeChannel(Channel* channel);
    // 阻塞等待事件，返回就绪的 Channel 指针列表
    std::vector<Channel*> poll(int timeout_ms = -1);

private:
    // 内部辅助函数，执行具体的 epoll_ctl 系统调用
    void update(int opertion, Channel* channel);
    
    int epoll_fd_;
    // 预分配好内存，避免每次 poll 都重新分配
    std::vector<struct epoll_event> active_events_;
};

#endif //EPOLL_H