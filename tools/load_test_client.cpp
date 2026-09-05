#include<iostream>
#include<atomic>
#include<chrono>
#include<string>
#include<thread>
#include<vector>
#include<cstring>
#include<unistd.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<fcntl.h>
#include<errno.h>
#include<algorithm>
#include<iomanip>
#include<netdb.h>

// 存放所有命令行参数和运行参数，统一管理
struct Config {
    std::string host = "127.0.0.1";
    int port = 8888;
    int concurrency = 100;
    int duration_seconds = 30;
    int message_size = 100;
    int timeout_seconds = 3;
};

// 使用 std::atomic 保证多个线程同时累加时数据不冲突
// total_bytes 记录收发总字节数，用于计算吞吐量
struct Stats {
    std::atomic<uint64_t> total_requests{0};
    std::atomic<uint64_t> total_errors{0};
    std::atomic<uint64_t> total_bytes{0};
};

class LoadTestClient {
public:
    explicit LoadTestClient(const Config& cfg);
    // 启动所有线程，等待结束，合并数据，打印报告
    void run();

private:
    // 建立 TCP 连接，设置超时，返回 fd
    int connectToServer();
    // 处理 partial send/recv
    bool sendFull(int fd, const std::string& msg);
    bool recvFull(int fd, std::string& out);
    // 每个线程的主循环，记录延迟到本地向量
    void worker(std::vector<double>& local_latencies);
    // 输出压测报告
    void printReport(const std::vector<double>& latencies, double qps, double throughput_mbps, double error_rate);
    
    Config cfg_;                    // 存储配置参数
    Stats stats_;                   // 存储统计结果（原子变量）
    std::atomic<bool> stop_{false}; //控制 worker 线程停止的标志

};

LoadTestClient::LoadTestClient(const Config& cfg) : cfg_(cfg) {}

int LoadTestClient::connectToServer() {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return -1;
    }

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(cfg_.port);

    if (inet_pton(AF_INET, cfg_.host.c_str(), &addr.sin_addr) <= 0) {
        close(fd);
        return -1;
    }

    if (connect(fd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        close(fd);
        return -1;
    }

    //设置收发超时
    struct timeval tv;
    tv.tv_sec = cfg_.timeout_seconds;
    tv.tv_usec = 0;
    //SO_SNDTIMEO：send 操作的超时时间。
    //SO_RCVTIMEO：recv 操作的超时时间。
    setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    
    return fd;

}

//确保完整发送
bool LoadTestClient::sendFull(int fd, const std::string& msg) {
    size_t sent_total = 0;
    while (sent_total < msg.size()) {
        ssize_t n = send(fd, msg.data() + sent_total, msg.size() - sent_total, 0);
        if (n <= 0) {
            return false;
        }
        sent_total += n;
    }
    return true;
}

// 确保完整接收
bool LoadTestClient::recvFull(int fd, std::string& out) {
    out.resize(cfg_.message_size);
    size_t received = 0;
    while (received < cfg_.message_size) {
        ssize_t n = recv(fd, &out[received], cfg_.message_size - received, 0);
        if (n <= 0) {
            return false;
        }
        received += n;
    }
    return true;
}


void LoadTestClient::worker(std::vector<double>& local_latencies){
    // 预分配空间，避免频繁扩容影响延迟测量
    local_latencies.reserve(10000);

    // 1. 建立连接
    int fd = connectToServer();
    if(fd == -1){
        stats_.total_errors++;
        return;
    }

    // 2. 准备请求数据（固定内容，避免每轮重复构造）
    std::string request(cfg_.message_size, 'A');
    std::string response;

    // 3. 循环执行，直到收到停止信号
    while(!stop_.load()){
        auto strat = std::chrono::steady_clock::now();

        // 发送请求
        if(!sendFull(fd, request)){
            stats_.total_errors++;
            break;
        }

        // 接收响应
        if(!recvFull(fd, response)){
            stats_.total_errors++;
            break;
        }

        auto end = std::chrono::steady_clock::now();
        double elapsed_us = std::chrono::duration<double, std::micro>(end - strat).count();

        // 校验响应内容是否正确
        if(response == request){
            stats_.total_requests++;
            stats_.total_bytes += request.size() + response.size();
            local_latencies.push_back(elapsed_us);
        }else{
            // 响应内容不匹配，说明服务器行为异常
            stats_.total_errors++;
            break;
        }
    }

    // 4. 关闭连接
    close(fd);

}

void LoadTestClient::run(){
    // 1. 打印启动信息
    std::cout << "=== Load Test Strating ===" << std::endl;
    std::cout << "Target: " << cfg_.host << ":" << cfg_.port << std::endl;
    std::cout << "Concurrency: " << cfg_.concurrency << std::endl;
    std::cout << "Duration: " << cfg_.duration_seconds << " seconds" << std::endl;
    std::cout << "Message size: " << cfg_.message_size << " bytes" << std::endl;
    std::cout << "Timeout: " << cfg_.timeout_seconds << " seconds" << std::endl;
    std::cout << "==========================" << std::endl;

    // 2. 准备每个线程的本地延迟存储
    std::vector<std::vector<double>> all_latencies(cfg_.concurrency);
    std::vector<std::thread> workers;
    workers.reserve(cfg_.concurrency);

    // 3. 启动所有工作线程
    for(int i = 0; i < cfg_.concurrency; i++){
        workers.emplace_back(&LoadTestClient::worker, this, std::ref(all_latencies[i]));
    }

    // 4. 等待指定时长
    std::this_thread::sleep_for(std::chrono::seconds(cfg_.duration_seconds));

    // 5. 通知所有线程停止
    stop_.store(true);

    // 6. 等待所有线程退出
    for(auto& t : workers){
        if(t.joinable()){
            t.join();
        }
    }

    // 7. 合并所有本地延迟数据
    std::vector<double> all;
    size_t total_samples = 0;
    for(auto& vec : all_latencies){
        total_samples += vec.size();
        all.insert(all.end(), vec.begin(), vec.end());
    }

    // 8. 排序（为百分位数计算做准备）
    std::sort(all.begin(), all.end());

    // 9. 计算核心指标
    //QPS	总请求数 / 压测时长（秒）
    double qps = static_cast<double>(stats_.total_requests.load()) / cfg_.duration_seconds;
    double total_mb = static_cast<double>(stats_.total_bytes.load()) / (1024.0 * 1024.0);
    //吞吐量（MB/s）	总字节数（MB）/ 压测时长（秒）
    double throughput_mbps = total_mb / cfg_.duration_seconds;
    uint64_t total_ops = stats_.total_requests.load() + stats_.total_errors.load();
    //错误率	总错误数 /（总请求数 + 总错误数）
    double error_rate = (total_ops > 0) ? (static_cast<double>(stats_.total_errors.load()) / total_ops) : 0.0;

    // 10. 打印报告
    printReport(all, qps, throughput_mbps, error_rate);

}

void LoadTestClient::printReport(const std::vector<double>& latencies, double qps, double throughput_mbps, double error_rate){
    // 1. 基本信息
    std::cout << "\n========== Load Test Report ==========" << std::endl;
    std::cout << "Target:           " << cfg_.host << ":" << cfg_.port << std::endl;
    std::cout << "Concurrency:      " << cfg_.concurrency << std::endl;
    std::cout << "Duration:         " << cfg_.duration_seconds << " seconds" << std::endl;
    std::cout << "Message size:     " << cfg_.message_size << " bytes" << std::endl;
    std::cout << "Timeout:          " << cfg_.timeout_seconds << " seconds" << std::endl;
    std::cout << "---------------------------------------" << std::endl;

    //
    double avg_lat = 0.0;
    double p50 = 0.0;
    double p99 = 0.0;

    if(!latencies.empty()){
        //
        double sum = 0.0;
        for(double v : latencies){
            sum += v;
        }
        avg_lat = sum / latencies.size();

        //
        size_t idx50 = static_cast<size_t>(latencies.size() * 0.50);
        size_t idx99 = static_cast<size_t>(latencies.size() * 0.99);

        if(idx99 >= latencies.size()) idx99 = latencies.size() - 1;
        p50 = latencies[idx50];
        p99 = latencies[idx99];

    }

    // 3. 计算总操作数
    uint64_t total_ops = stats_.total_requests.load() + stats_.total_errors.load();

    // 4. 打印统计结果
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Total requests:   " << stats_.total_requests.load() << std::endl;
    std::cout << "Total errors:     " << stats_.total_errors.load() << std::endl;
    std::cout << "QPS:              " << qps << std::endl;

    if (!latencies.empty()) {
        std::cout << "Avg latency:      " << avg_lat / 1000.0 << " ms" << std::endl;
        std::cout << "P50 latency:      " << p50 / 1000.0 << " ms" << std::endl;
        std::cout << "P99 latency:      " << p99 / 1000.0 << " ms" << std::endl;
    } else {
        std::cout << "Avg latency:      N/A (no successful requests)" << std::endl;
        std::cout << "P50 latency:      N/A" << std::endl;
        std::cout << "P99 latency:      N/A" << std::endl;
    }

    std::cout << "Throughput:       " << throughput_mbps << " MB/s" << std::endl;
    std::cout << "Error rate:       " << error_rate * 100.0 << " %" << std::endl;
    std::cout << "=======================================" << std::endl;
}

void printUsage(const char* prog) {
    std::cout << "Usage: " << prog << " [concurrency] [duration_seconds] [message_size]" << std::endl;
    std::cout << "  concurrency       Number of concurrent connections (default: 100)" << std::endl;
    std::cout << "  duration_seconds  Test duration in seconds (default: 30)" << std::endl;
    std::cout << "  message_size      Size of each message in bytes (default: 100)" << std::endl;
    std::cout << "Example: " << prog << " 500 30 100" << std::endl;
}

int main(int argc, char* argv[]) {
    Config cfg;

    // 1. 解析命令行参数
    if (argc >= 2) {
        int c = std::stoi(argv[1]);
        if (c > 0) cfg.concurrency = c;
        else { std::cerr << "Invalid concurrency: " << argv[1] << std::endl; return 1; }
    }
    if (argc >= 3) {
        int d = std::stoi(argv[2]);
        if (d > 0) cfg.duration_seconds = d;
        else { std::cerr << "Invalid duration: " << argv[2] << std::endl; return 1; }
    }
    if (argc >= 4) {
        int s = std::stoi(argv[3]);
        if (s > 0) cfg.message_size = s;
        else { std::cerr << "Invalid message size: " << argv[3] << std::endl; return 1; }
    }
    if (argc > 4) {
        printUsage(argv[0]);
        return 1;
    }

    // 2. 运行压测
    LoadTestClient client(cfg);
    client.run();

    return 0;
}
