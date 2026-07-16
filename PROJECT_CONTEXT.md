# 个人学习项目完整上下文文档

> **文档用途**：供 Agent 知识库加载，完整理解用户的技术背景、项目进度、开发环境和协作方式。
> **最后更新**：2026年7月16日


## 一、用户身份与背景

### 1.1 基本信息
- **身份**：广东技术师范大学，计算机专业，大二升三学生
- **技术方向**：C/C++ 底层开发
- **目标行业**：用户曾提及对以下行业感兴趣：车载、游戏、安防、边缘AI、金融科技。但该列表仅反映当时关注的方向，不作为硬性目标。

### 1.2 初始学习履历（项目开始前，2026年6月底）

#### C/C++ 学习线
- 入门阶段：完整学完翁恺 C、C++ 全套课程，配套手写笔记；完成 100+ 校内基础语法练习题（非 LeetCode 算法题）
- 实操课程：完整看完黑马《C++从入门到精通》，跟随课程敲写多个小型基础 Demo 项目
- 书籍进度：《C++ Primer Plus》阅读 400 页后中断，后半本 STL、模板、泛型、异常等内容未学习
- 高阶面向对象：侯捷《C++面向对象高级开发》上下全集完整学完
- 内存专题：侯捷内存管理课程因难度过高中途弃坑，智能指针、底层内存分配、内存泄漏排查等内容空白

#### 数据结构与算法
- 基础数据结构：完整学习浙大陈越数据结构课程，可从零简易手写链表、栈、队列、二叉树、图、各类排序查找；仅实现基础增删改查，无泛型封装、边界容错、线程安全处理
- 面试刷题：《代码随想录》刷题进度约 30%，尚未系统刷完 DP、贪心、图论等高频面试题型

#### 计算机底层系统
- 《深入理解计算机系统（CSAPP）》全书通读完毕，记录少量笔记；配套 DataLab、BombLab、BufferLab 等全部实验未动手实操

#### 专业基础课程
- 计算机网络：完整学习湖科大计算机网络全套课程，掌握 TCP/IP 分层、HTTP 等基础理论
- 数据库：自学起步阶段，仅掌握基础 SQL 增删改查，索引、事务、锁、存储引擎、SQL 优化等底层内容未学习

### 1.3 核心特征（学习风格）
- 通过写代码倒逼理解，而非先看书再动手
- 对“先搞清楚再写”和“先写再回头看”两种方式都能接受，但后者更符合当前节奏
- 自我认知清晰，能精准列出短板
- 执行力在实战中已验证（连续三周保持每日产出）


## 二、完整学习规划

### 2.1 总体目标（2026年暑假结束时的硬指标）
- **项目1**：`LinkedList<T>` 完整容器库（含迭代器、深拷贝、Stack/Queue 适配器）✅ 已完成
- **项目2**：三种并发模型的 Echo Server（单线程 + 多进程 + 多线程）✅ 已完成
- **项目3**：基于 Epoll + 线程池的 Reactor 模式高并发 Echo Server（⏳ 第4周）
- **实验1**：CSAPP BombLab（反汇编 + GDB 调试实战）（⏳ 第5周）
- **实验2**：CSAPP MallocLab（内存分配器实现，得分≥80/100）（⏳ 第6周）
- **面试准备**：C++ 八股文笔记 + 一页简历 + 3次模拟面试（⏳ 第7-8周）
- **GitHub**：每日 commit，远程仓库持续更新

### 2.2 完整时间轴

| 阶段 | 周次 | 日期范围 | 主题 | 状态 |
| :--- | :--- | :--- | :--- | :--- |
| 阶段一 | 第1周 | 6.28 - 7.4 | 基础工具链收尾（容器库 + 热身） | ✅ 已完成 |
| 阶段二 | 第2周 | 7.5 - 7.11 | 阻塞式网络编程入门（三种并发模型） | ✅ 已完成 |
| 阶段三 | 第3周 | 7.12 - 7.18 | Epoll 核心 + 单线程事件驱动 | ✅ 已完成 |
| 阶段四 | 第4周 | 7.16 - 7.22 | 线程池 + Reactor 模式整合 | ⏳ 进行中 |
| 阶段五 | 第5周 | 7.23 - 7.29 | CSAPP BombLab | ⏳ 待执行 |
| 阶段六 | 第6周 | 7.30 - 8.5 | CSAPP MallocLab | ⏳ 待执行 |
| 阶段七 | 第7周 | 8.6 - 8.12 | 面试笔记整理 + 项目文档 | ⏳ 待执行 |
| 阶段八 | 第8周 | 8.13 - 8.19 | 简历 + 模拟面试 + 投递 | ⏳ 待执行 |
| 缓冲期 | — | 8.20 - 8.31 | 查漏补缺 + 面试跟进 | ⏳ 待执行 |

### 2.3 各阶段详细任务

#### 第1周（已完成）：基础工具链收尾
- LinkedList 模板化（`template<typename T>`）
- 迭代器支持（范围 `for` 循环）
- Stack 适配器 + Queue 适配器
- 所有代码通过 ASan 内存检测

#### 第2周（已完成）：三种并发模型的 Echo Server
- 单线程阻塞版（`echo_server.cpp`）
- 多进程版（`echo_server_fork.cpp`）
- 多线程版（`echo_server_pthread.cpp`）
- 信号处理（sigaction 优雅退出）
- send 循环修复（防止部分发送）

#### 第3周（已完成）：Epoll 高并发入门
- Epoll 最小骨架（`epoll_server_skel.cpp`）
- LT 模式 Epoll Echo Server（`epoll_server_lt.cpp`）
- ET 模式 Epoll Echo Server（`epoll_server_et.cpp`）
- 输出缓冲区 + EPOLLOUT 支持（`epoll_server_buffer.cpp`）
- 压力测试通过（100MB MD5 验证 + 100 空闲连接 + 1000 次反复连接）

#### 第4周（进行中）：线程池 + Reactor 模式
- 实现固定大小线程池（任务队列 + 互斥锁 + 条件变量）
- 将 Epoll 事件处理函数打包为任务，提交给线程池执行
- 处理并发读写冲突（每个 fd 的读写操作加锁）
- 压力测试验证

#### 第5周（待执行）：CSAPP BombLab
- 下载 `bomb.tar`，环境准备
- 拆除 Phase 1-6
- 整理解题笔记

#### 第6周（待执行）：CSAPP MallocLab
- 阅读框架代码，实现隐式空闲链表
- 实现显式空闲链表（双向链表）
- 实现分离适配（Segregated Fit）
- 实现 `mm_realloc` + 性能调优（`mdriver` 得分 ≥ 80/100）

#### 第7-8周（待执行）：面试冲刺
- 整理面试笔记（C++ / 计网 / OS / 数据库）
- 撰写项目 README
- 撰写一页简历
- 模拟面试 3 次
- 投递实习（Boss直聘 / 实习僧）


## 三、开发环境与工具链

### 3.1 虚拟机环境
- **操作系统**：Ubuntu 24.04.4 LTS（桌面版）
- **内核版本**：6.8.0-111-generic
- **工作目录**：`/home/hhygbd/cpp-summer`（全小写）
- **网络**：NAT 模式，可访问外网，127.0.0.1 回环可用

### 3.2 编译器与工具链
- **GCC**：13.3.0（支持 C++17 标准）
- **GDB**：15.1（调试器）
- **CMake**：3.22.1（构建工具）
- **Make**：4.3
- **编译命令模板**：`g++ -std=c++17 -g -fsanitize=address <源文件> -o <可执行文件>`
- **链接选项**：多线程程序需加 `-pthread`

### 3.3 编辑器与调试
- **编辑器**：VSCode（虚拟机内本地使用，非 Remote-SSH）
- **VSCode 扩展**：C/C++（ms-vscode.cpptools）、CMake Tools（可选）
- **调试配置**：`.vscode/launch.json` 和 `.vscode/tasks.json` 已配置
  - 按 F5 启动调试，自动触发 `cmake --build build` 编译
  - 调试目标：`${workspaceFolder}/build/summer_app`
- **启动方式**：在虚拟机桌面打开 VSCode，`File → Open Folder` 选中 `~/cpp-summer`

### 3.4 Git 与版本控制
- **本地仓库**：`~/cpp-summer`（已初始化）
- **远程仓库**：`https://github.com/hhygbd/cpp-summer`
- **Git 配置**：
  - `user.email`：`1478339176@qq.com`
  - `user.name`：`HHYGBD`
- **提交习惯**：每日至少 1 次 commit，按功能点提交
- **推送方式**：`git push`，使用 Personal Access Token 鉴权


## 四、代码仓库文件结构

### 4.1 根目录结构
```
~/cpp-summer/
├── .vscode/                     # VSCode 调试配置
│   ├── launch.json              # 调试启动配置
│   └── tasks.json               # 编译任务配置
├── build/                       # CMake 构建目录（当前未使用）
├── src/                         # 所有源码
│   ├── linkedlist.hpp           # 模板链表容器
│   ├── stack.hpp                # 栈适配器
│   ├── queue.hpp                # 队列适配器
│   ├── echo_server.cpp          # 单线程 Echo Server
│   ├── echo_server_fork.cpp     # 多进程 Echo Server
│   ├── echo_server_pthread.cpp  # 多线程 Echo Server
│   ├── epoll_server_skel.cpp    # Epoll 骨架（仅监听连接）
│   ├── epoll_server_lt.cpp      # LT 模式 Epoll Server
│   ├── epoll_server_et.cpp      # ET 模式 Epoll Server
│   ├── epoll_server_buffer.cpp  # ET + 输出缓冲区 + EPOLLOUT
│   ├── test_list.cpp            # 链表测试文件
│   └── test_adapter.cpp         # Stack/Queue 测试文件
├── algorithms/                  # 算法刷题代码（新建）
│   └── leetcode/
│       └── string/
│           └── kmp.cpp          # KMP 算法实现
├── notes/                       # 笔记目录（建议新建）
│   └── interview_八股_计网_计组_数据库.md  # 八股文复习手册
├── CMakeLists.txt               # CMake 配置
└── README.md                    # 项目说明
```

### 4.2 当前活跃文件（Agent 应关注的）
| 文件 | 用途 | 状态 |
| :--- | :--- | :--- |
| `epoll_server_buffer.cpp` | 当前最新版本 Epoll 服务器 | ✅ 已通过压力测试 |
| `src/` 目录下所有文件 | 项目源码 | ✅ 均已提交 Git |

### 4.3 编译与运行命令（Agent 必须知道）
```bash
# 编译任意版本
g++ -std=c++17 -g -fsanitize=address src/<文件名>.cpp -o <可执行文件名>

# 运行服务器
./<可执行文件名>

# 测试连接（另开终端）
telnet 127.0.0.1 8888

# 查看进程 fd 数量
ls -l /proc/<PID>/fd | wc -l
```


## 五、当前能力状态（2026年7月16日）

### 5.1 已掌握的能力

#### C++ 工程能力
- 能用 `class` 封装数据结构，手写构造函数、析构函数，实现深拷贝（拷贝构造 + 拷贝赋值运算符）
- 能用 `new/delete` 管理动态内存，通过 ASan 检测内存泄漏
- 能写 `template<typename T>` 泛型类，使容器支持不同类型
- 能为容器编写内部迭代器类，重载 `operator*`、`operator++`、`operator!=`，支持范围 `for` 循环
- 能用已有容器作为底层，封装出 `Stack<T>` 和 `Queue<T>` 适配器类
- 代码组织：能用 `#ifndef/#define/#endif` 头文件保护

#### Linux 网络编程
- **基础 Socket API**：能独立完成 `socket/bind/listen/accept/recv/send/close` 全流程，设置 `SO_REUSEADDR`，处理 `telnet` 测试
- **多进程并发**：能用 `fork()` 实现并发，用 `signal(SIGCHLD, SIG_IGN)` 防止僵尸进程
- **多线程并发**：能用 `pthread_create()` 和 `pthread_detach()`，能解决线程函数传参问题（堆内存 `new int(client_fd)`）
- **Epoll I/O 多路复用**：能用 `epoll_create1/epoll_ctl/epoll_wait`，能区分 LT 和 ET 模式并分别实现，能用 `fcntl()` 设置非阻塞，能处理 `EAGAIN`/`EINTR`
- **输出缓冲区与 EPOLLOUT**：能用 `unordered_map<int, string>` 管理输出缓冲区，能处理 `send` 返回 `EAGAIN` 时的数据缓存与续发
- **信号处理**：能用 `sigaction` 注册 `SIGINT` 实现优雅退出

#### Git 与版本控制
- 能独立完成 `git add/commit/push` 完整流程
- 能关联本地仓库与 GitHub 远程仓库，使用 Token 完成推送

### 5.2 当前未涉及的内容（待学/待补）
- 移动构造 / 移动赋值语义
- 智能指针（`unique_ptr` / `shared_ptr` / `weak_ptr`）
- C++ 异常处理（`try/catch`）
- CSAPP BombLab（反汇编 + GDB 调试）
- CSAPP MallocLab（内存分配器）
- 数据库相关知识（索引、事务、MVCC）
- 面试八股文系统复习（TCP 状态机、TIME_WAIT、虚拟内存、页表、B+ 树等）

### 5.3 理论差距清单（按优先级）
| 优先级 | 模块 | 需补内容 |
| :--- | :--- | :--- |
| 高 | TCP / 网络编程原理 | TCP 状态机、TIME_WAIT、select/poll/epoll 对比 |
| 高 | 进程 / 内存管理 | 虚拟内存、页表、堆栈、fork 的 COW |
| 中 | 数据库索引与事务 | B+ 树、聚簇/非聚簇索引、隔离级别、MVCC |
| 中 | C++ 智能指针 / 移动语义 | `unique_ptr/shared_ptr/weak_ptr`、移动构造 |
| 低 | 数据库日志 | `redo/undo/binlog` |


## 六、Agent 协作约定

### 6.1 任务交接方式
- 用户会在新对话中提供本次交接文档
- Agent 应首先确认已理解文档内容，然后询问用户想继续哪个方向
- 每个编码任务应以**交接文档**形式输出，包含：任务目标、具体步骤、验收标准、测试命令

### 6.2 Agent 职责
- 基于文档理解用户的精确进度，不假设用户已经掌握未列出的能力
- 安排任务时，明确说明当前在整体计划中的位置（第几周、第几天）
- 遇到用户贴报错时，直接定位问题根源，不要求用户重述背景
- 不替用户做决定，提供选项并说明利弊，由用户选择

### 6.3 当前会话状态
- 用户刚完成第3周全部任务（Epoll + 压力测试），准备进入第4周（线程池 + Reactor）
- 如需继续推进，请询问用户接下来想做什么，或直接给出第4周任务选项

### 6.4 强制约定
- 每次新对话启动时，Agent 应确认已加载本上下文文档，再开始任何任务安排


## 七、文档维护说明

- **更新频率**：每个编码任务完成后更新
- **更新内容**：已完成任务标记、新增文件、能力状态变化
- **版本管理**：与 Git 仓库同步，每次更新后建议 `git add` 本文件

