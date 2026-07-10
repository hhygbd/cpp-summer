#include <iostream>
#include <vector>
#include <string>   // 必须显式包含，保证跨平台编译通过

// 构建 next 数组
void getNext(int* next, const std::string& s) {
    int m = (int)s.size();  // 转为 int，避免无符号带来的回退隐患
    if (m == 0) return;

    int j = -1;
    next[0] = -1;
    for (int i = 1; i < m; i++) {
        while (j >= 0 && s[i] != s[j + 1]) { // 前后缀不相等
            j = next[j];
        }
        if (s[i] == s[j + 1]) { // 找到相同前后缀
            j++;
        }
        next[i] = j; // 更新next数组
    }
}

// 匹配函数
int match(int* next, const std::string& haystack, const std::string& needle) {
    int n = (int)haystack.size();
    int m = (int)needle.size();

    if (m == 0) return 0;   
    if (n < m) return -1;   // 剪枝优化

    int j = -1; // next数组的起始记录也为-1
    for (int i = 0; i < n; i++) {
        while (j >= 0 && haystack[i] != needle[j + 1]) {
            j = next[j];
        }
        if (haystack[i] == needle[j + 1]) {
            j++;
        }
        if (j == m - 1) {       
            return i - m + 1;   
        }
    }
    return -1;
}

// 主调用接口
int strStr(std::string haystack, std::string needle) {
    // 先拦截空串，防止 vector 容量为 0 时传 &next[0] 导致崩溃
    if (needle.empty()) return 0;

    int m = (int)needle.size();
    std::vector<int> next(m);
    getNext(next.data(), needle);  // C++11 data() 代替 &next[0]
    return match(next.data(), haystack, needle);
}