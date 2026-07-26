#ifndef CHANNEL_H
#define CHANNEL_H
#include<functional>
#include<sys/epoll.h>

class Channel{
public:
    // 使用 std::function 存储回调，支持普通函数、lambda、类成员函数绑定
    using EventCallback = std::function<void()>;
    explicit Channel(int fd);
    ~Channel() = default;
    // 设置各种事件的回调
    void setReadCallback(EventCallback cb){ readCallback_ = std::move(cb); }
    void setWriteCallback(EventCallback cb){ writeCallback_ = std::move(cb); }
    void setErrorCallback(EventCallback cb){ errorCallback_ = std::move(cb); }
    // 动态修改关注的事件（修改后需要调用 Epoll::updateChannel 生效）
    void enableReading(){ events_ |= EPOLLIN; }
    void enableWriting(){ events_ |= EPOLLOUT; }
    void disableWriting(){ events_ &= ~EPOLLOUT; }
    void disableAll(){ events_ = 0; }
     // 供 Epoll 类调用的接口
    int Get_fd() const { return fd_; }
    uint32_t Get_events() const { return events_; }
    void setRevents(uint32_t revt){ revents_ = revt; }
    // 核心逻辑：根据 epoll 返回的实际事件，触发对应的回调
    void handleEvent();

    void setIndex(int index) { index_ = index; }
    int index() const { return index_; }

private:
    int fd_;
    int index_; // 约定：-1 表示未添加（新创建），1 表示已添加到 epoll
    uint32_t events_;// 关注的事件 (如 EPOLLIN | EPOLLOUT)
    uint32_t revents_;// epoll_wait 返回的实际发生的事件
    EventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback errorCallback_;
};

#endif