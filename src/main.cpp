#include <iostream>
#include <memory>
#include <vector>
#include <string>

class ScopedLogger {
public:
    explicit ScopedLogger(const std::string& name) : name_(name) {
        std::cout << "[+] Entering: " << name_ << std::endl;
    }
    ~ScopedLogger() {
        std::cout << "[-] Exiting: " << name_ << std::endl;
    }
private:
    std::string name_;
};

int main() {
    std::cout << "=== C++ Summer Env Check ===" << std::endl;
    {
        ScopedLogger log("Main Scope");
        std::vector<int> test_vec = {1, 2, 3, 4, 5};
        std::cout << "Vector size: " << test_vec.size() << std::endl;
    }
    auto ptr = std::make_unique<int>(42);
    std::cout << "Unique_ptr value: " << *ptr << std::endl;
    std::cout << "Ready to work!" << std::endl;
    return 0;
}
