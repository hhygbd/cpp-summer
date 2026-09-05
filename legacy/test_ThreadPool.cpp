#include<iostream>
#include<thread>
#include<chrono>
#include<mutex>
#include"ThreadPool.h"
std::mutex print_mutex;
int main(){

    ThreadPool pool(4);
    
    for(int i = 0; i < 8; i++){
        pool.addTask([&print_mutex](int task_id) {
            {

            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "Task " << task_id << " running on thread " << std::this_thread::get_id() << std::endl;

            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            
        }, i); 
        
    }
    std::cout << "Main thread finished." << std::endl;

    return 0;
}