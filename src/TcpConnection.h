#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <string>
#include <memory>

class TcpConnection {
public:
    explicit TcpConnection(int fd);
    ~TcpConnection();

    // 处理 EPOLLIN 事件（循环 recv）
    // 返回值：true 表示连接正常，false 表示连接断开或出错（需要关闭）
    bool handleRead();

    // 处理 EPOLLOUT 事件（循环 send 缓冲区数据）
    // 返回值：true 表示连接正常，false 表示连接断开或出错
    bool handleWrite();

    // 将数据追加到输出缓冲区
    void appendOutputBuffer(const std::string& data);

    // 获取文件描述符
    int Get_fd() const;

    // 检查输出缓冲区是否有待发送数据
    bool hasOutputData() const;

    // 禁用拷贝
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

private:
    int fd_;
    std::string input_buffer_;  // 接收缓冲区（目前 Echo 服务器可能用不到，但标准设计必须有）
    std::string output_buffer_; // 发送缓冲区（替代你原来的全局 output_buffers）
};

// 使用智能指针管理连接生命周期
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

#endif // TCP_CONNECTION_H