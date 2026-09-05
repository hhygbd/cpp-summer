#include"ThreadPool.h"
#include<iostream>

ThreadPool::ThreadPool (size_t thread_count):stop_(false){
        //防御性编程，如果传入 0 ，无工作线程，任务会永远卡在队列里
        if(thread_count == 0){
            thread_count = std::thread::hardware_concurrency();//CUP逻辑核心数
            //防御性编程，获取失败
            if(thread_count == 0){
                    thread_count = 4;//保底
            }
        }

        for(size_t i = 0; i < thread_count; ++i){

            workers_.emplace_back(&ThreadPool::workerLoop,this);

        }
}

void ThreadPool::workerLoop(){
        while(!stop_ || !tasks_.empty()){
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                // 1. 阻塞等待。
                // wait 的第二个参数（predicate）内部自带 while 循环，已经完美处理了虚假唤醒。
                // 只有当 stop_ 为 true，或者 tasks_ 不为空时，才会真正返回。
                condition_.wait(lock,[this](){return stop_ || !tasks_.empty();});
                // 2. 优雅退出判断。
                // 如果线程池正在停止，且队列里的任务已经全部处理完毕，则结束当前工作线程。
                if(stop_ && tasks_.empty()){
                    return;
                }
                // 3. 安全取出任务。
                // tasks_ 必然不为空。使用 std::move 避免 std::function 内部可能的堆内存拷贝。
                task = std::move(this->tasks_.front());
                tasks_.pop();
            }// 离开大括号作用域，unique_lock 析构，自动释放 queue_mutex_
            
            // 4. 无锁执行任务。
            // 在锁外执行，确保其他工作线程可以并发地获取任务。
            if(task){
                task();
            }

        }
}

ThreadPool::~ThreadPool(){
    stop_ = true;
    condition_.notify_all();
    for(std::thread& worker : workers_){
        if(worker.joinable()){//返回 true 表示这个线程还在运行，且可以被等待
            worker.join();
        }
    }
}