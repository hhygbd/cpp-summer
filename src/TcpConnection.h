#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include"EventLoop.h"
#include"Channel.h"
#include<atomic>
#include<string>
#include<memory>
#include<functional>

//继承——enable_shared_from_this,以便在回调的过程中安全传递this的智能指针
class TcpConnection : public std::enable_shared_from_this<TcpConnection>{
public:

    //构造函数需要传入所属的EventLoop
    TcpConnection(EventLoop* loop, int fd);
    ~TcpConnection();

    // ----- 跨线程生命周期管理（由 TcpServer 调用）-----
    //建立连接时调用：将 Channle 注册到所属的 EventLoop
    void connectEstablished();
    //销毁连接时调用：将 Channle 从 EventLoop 注销
    void connectDestroyed();

    // ----- 线程安全的发送接口 -----
    //可在任意线程调用，内部会转发到所属的 EventLoop 执行
    void send(const std::string& msg);

    //强制关闭连接（线程安全)
    void forceClose();
    
    // ----- 查询接口 -----
    int fd() const{ return fd_; }
    EventLoop* getLoop() const{ return loop_; } 


    // ----- 禁用拷贝 -----
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

private:
    // ----- 实际在所属 EventLoop 线程中执行的函数 -----
    void connectEstablishedInLoop();
    void connectDestroyedInLoop();
    void sendInLoop(const std::string& msg);
    void forceCloseInLoop();

    // ----- Channel 事件回调（绑定到 Channel 上）-----
    void handleRead();
    void handleWrite();
    void handleError();
    void handleClose();// 处理对端关闭或严重错误

    // ----- 连接状态管理 -----
    enum StateE { kConnecting, kConnected, kDisconnecting, kDisconnected };
    void SetState(StateE s) { state_.store(s); }

    // ----- 成员变量 -----
    EventLoop* loop_; // 当前连接所属的 EventLoop（子线程）
    int fd_;
    std::unique_ptr<Channel> channel_;// 接管 Channel 的所有权

    std::string input_buffer_;
    std::string output_buffer_;

    std::atomic<StateE> state_;// 线程安全的连接状态

};

// 使用智能指针管理连接生命周期
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

#endif // TCP_CONNECTION_H