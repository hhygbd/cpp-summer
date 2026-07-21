#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include<queue>
#include<vector>
#include<condition_variable>
#include<future>
#include<thread>
#include<atomic>
#include<mutex>
#include<stdexcept>
#include<memory>
#include<functional>

class ThreadPool{
public:
    // 构造函数：初始化指定数量的工作线程
    explicit ThreadPool (size_t thread_count);
    // 析构函数：实现“优雅退出”
    ~ThreadPool();
    // 核心接口：提交任务
    // 使用模板和完美转发，支持任意函数和任意参数
    // 返回 std::future，允许调用者获取任务的返回值
    template<class F,class... Args>
    auto addTask(F&& f, Args&&... args)
        ->std::future<typename std::result_of<F(Args...)>::type>;
    // 禁用拷贝和赋值，线程池应该是单例或唯一持有的资源
    ThreadPool(const ThreadPool&)=delete;
    ThreadPool& operator()(const ThreadPool)=delete;

private:
    // 工作线程的主循环函数
    void workerLoop();
    // 1. 线程管理
    std::vector<std::thread> workers_;
    // 2. 任务队列
    std::queue<std::function<void()>> tasks_;
    // 3. 同步与状态控制
    std::mutex queue_mutex_;
    std::condition_variable condition_;

    // 停止标志。使用 atomic 保证在锁外读取时的线程安全
    std::atomic<bool> stop_;

};
//模板函数实现 (必须放在 .h 中)
template<class F,class... Args>
auto ThreadPool::addTask(F&& f, Args&&... args)
    ->std::future<typename std::result_of<F(Args...)>::type>{
        // 1. 检查线程池是否已停止
        if(stop_){
            throw std::runtime_error("addTask on stopped ThreadPool");
        }
        // 2. 推导返回值类型
        using return_type = typename std::result_of<F(Args...)>::type;
        // 3. 将任务和参数打包，并包装到 std::packaged_task 中以支持 std::future
        std::shared_ptr<std::packaged_task<return_type()>> task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        // 4. 获取 future 对象，用于将来获取结果
        std::future<return_type> res = task->get_future();
        // 5. 入队
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // 将 packaged_task 包装为无参的 std::function<void()> 放入队列
            tasks_.emplace([task]() { (*task)(); });
        }// 离开作用域，自动释放锁
        // 6. 唤醒一个工作线程
        condition_.notify_one();
        return res;
    }

#endif// THREAD_POOL_H