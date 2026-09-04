# myminishell

## 项目简介

这是一个基于Linux用户态实现的MiniShell,主要用来实现命令解析、进程控制、管道、重定向、Job Control、Signal/Event等功能,以此深入学习Linux系统编程相关机制🙃
项目目前已逐步增加了模块化、配置、日志、错误处理、Job管理等多项工程化能力.后续V1.6开始迁移至ARM Linux/Orange Pi环境,并逐渐向嵌入式Linux设备管理终端方向扩展.

## 开发环境

- OS: Ubuntu 22.04
- compiler: GCC
- 编译工具: Make
- 编程语言: C11

## 当前版本:V1.5 Engineering Release

## 更新日志

V1.5:
- 优化signal/event/terminal/job  control
- 修缮资源生命周期管理
- 引入自动化测试/Sanitizer/Valgrind
- 新增CI/arm linux交叉编译链

V1.4:
- 增加process groups
- 优化terminal与job contro
- 增加fg与bg等系统操作功能

V1.3:
- 优化工程稳定性
- 优化job/event/executor/shell/errno层代码

V1.2:
- 新增system_info模块,识别设备状态
- 优化原有config/logger文件实现

V1.1:
- 新增builtin_table文件,优化原有代码逻辑
- table 框架新增识别 help/job/status 功能
- 优化原有状态管理

V1.0:
- 完成MiniShell工程化重构
- 新增Config配置管理模块
- 新增Error错误处理模块
- 新增Log日志系统
- 新增基础框架测试

V0.9.3:
- 完善select构建基础事件循环
- 完善ShellContext统一管理 Shell 状态

V0.9.2:
- 引入事件驱动模型
- 实现select监听多路
- 重构sigchld处理流程

V0.9.1:
- 重构Shell生命周期控制机制,实现Shell运行状态统一管理
- 优化exit退出流程，实现状态驱动退出

V0.9:
- 初步开始MiniShell工程化重构
- 优化模块职责划分,提高系统可扩展性

V0.8.2:
- 新增job_list/job_remove函数,基本实现后台任务管理
- 优化原有sigchld函数实现

V0.8.1:
- 引入job,记录运行数据信息
- 初步完成后台任务管理

V0.8:
- 新增后台管理机制
- 支持&符号后台执行

V0.7.4:
- 新增test.md文件,用以初步检测现有指令解析处理功能

V0.7.3:
- 暂时注释sigchld,减少进程回收冲突
- 新增Ctrl + D 指令实现,优化Ctrl + C指令实现
- 新增内存释放函数,修补缺失功能

V0.7.2:
- 完善$?机制,完善shell功能
- 修复pipeline返回值

V0.7.1:
- 修补代码逻辑漏洞,修复原有追加重定向缺失

V0.7:
- 优化execute进程处理逻辑,实现返回退出码
- 新增shell_context文件,处理shellstatus值

V0.6.2:
- 优化signal机制,引入SIGCHLD
- 调整原有execute函数进程逻辑

V0.6.1:
- 更新sig函数,优化信号处理方式
- 实现父子进程信号区别接受处理

V0.6:
- 引入signal机制
- 实现 Ctrl + C 指令功能

V0.5.3:
- 优化原有命令管道解析,升级为可识别多重管道
- 优化executor文件逻辑实现

V0.5.2:
- 剥离redirect，优化代码逻辑

V0.5.1:
- 优化原有executor解析函数
- 完整实现pipe管道与指令分析功能

V0.5:
- 新增识别 "|" ,优化符号识别
- 引入管道pipe,解析多重指令
- 修改原有parse函数类型为结构体

- V0.4.2:
- 继续优化tokenize函数,引入多种符号检测
- 持续精简模块,剔除无用逻辑

- V0.4.1:
- 新增识别">>"符号,持续优化识别逻辑

- V0.4:
- 新增command文件,新增command结构体取代原有指令解析结构体
- 新增重定向部分,可识别"<>"等符号,优化命令解析逻辑

- V0.3.1
- 编写cd/pwd/exit等初等内部指令,完善builtin文件
- 完善dispatcher逻辑,补全细节
- 测试运行

- V0.3:
- 新增dispatcher骨架,优化原有判断逻辑
- 新增builtin内建命令解析函数,区分原有execute函数

- V0.2.1:
- 新增tokenize,取缔原有strtok函数,优化指令解析能力
- 新增 Token/TokenList 结构体,引入工程化模板
- 引入execvp函数,正式开始执行进程shell

- V0.2:
- 新增parser文件,用以解析指令输入
- 实现初步切割指令,识别exit等基础指令

- V0.1:初始化项目,初步构建MiniShell生命周期框架



## 命令执行

当前版本支持:

```
普通外部命令
单命令执行
多级 Pipeline
输入重定向 <
输出重定向 >
追加重定向 >>
后台执行 &
内建命令
```

示例:

```
ls -l
echo hello > output.txt
echo world >> output.txt
cat < output.txt
printf "a\nb\nc\n" | grep . | wc -l
sleep 10 &

当前内建命令:
cd
pwd
exit
jobs
fg
bg
help
status
sysinfo
reload
```

## 项目主要核心机制

### Job Control
```
miniShell已实现基础 unix Job Control

主要能力:
- process group control
- pgid 管理
- foreground/background Job
- `Ctrl+C`
- `Ctrl+Z`
- `fg`
- `bg`
- `jobs`
- 前台终端所有权切换
- STOPPED / RUNNING / DONE 状态管理
- Pipeline 整体作为一个 Job 管理
- Shell 退出时后台 Job 清理

Job 状态:
- JOB_RUNNING
- JOB_STOPPED
- JOB_DONE

单个 Process 状态：
- PROCESS_RUNNING
- PROCESS_STOPPED
- PROCESS_DONE
```

### signal/event
```
miniShell不在signal handler中执行复杂逻辑
当前工作流程:

signal
   |
sig_atomic_t pending event
   |
self-pipe
   |
select()
   |
normal process context
   |
job_reap / prompt handling

处理的主要事件包括：

- SIGCHLD
- SIGINT

Shell 自身忽略 Job Control 相关终端信号:

- SIGQUIT
- SIGTSTP
- SIGTTIN
- SIGTTOU

子进程在 execvp() 前恢复默认 signal disposition

```

### event loop
```
Shell 主循环使用:

- select()

同时监听:

- STDIN
- Event self-pipe

```

## 配置系统
默认配置文件:
```
config/config.conf

```

当前默认配置:

```
prompts=MiniShell
max_job=64
debug=0
```
---

### system info

sysinfo用于读取当前系统状态,包含:

```
Kernel
Hostname
Architecture
CPU
Memory
Uptime
```

完整模块详细设计见以下文件:

```
docs/architecture.md
docs/module.md
```
## 运行说明
开发环境:

```
Linux/Ubuntu 22.04
GCC
C11
Visual Studio Code (ssh)
GNU Make
```

默认构建:

```
make
```

默认生成:

```
build/default/minishell
./shell
```
编译Shell:

```
Makefile
make
./shell
```
编译测试:

```
Makefile
make test
./test
```

运行操作:

```
./shell
make run
```
## build 方式

普通 debug build:
```
make debug
```
Release Build:
```
make release
```
严格编译:
```
make strict
```
严格编译会启用以下机制:
```
-Wall
-Wextra
-Wpedantic
-Wformat=2
-Werror
```
## Test说明

完整普通测试:

```
make check
```

Unit Test:

```
make test
```

Integration Test:

```
make integration
```
详细测试说明见以下文件:

```
docs/test.md
```




## 项目结构

```
.
├── CMakeLists.txt
├── Makefile
├── README.md
├── common
│   ├── error.c
│   ├── error.h
│   ├── log.c
│   ├── log.h
│   ├── utils.c
│   └── utils.h
├── config
│   ├── config.c
│   ├── config.conf
│   └── config.h
├── docs
│   ├── architecture.md
│   ├── debug_log.md
│   ├── module.md
│   └── test.md
├── include
│   ├── builtin.h
│   ├── builtin_table.h
│   ├── command.h
│   ├── dispatcher.h
│   ├── event.h
│   ├── executor.h
│   ├── job.h
│   ├── parser.h
│   ├── shell.h
│   ├── shell_context.h
│   ├── sig.h
│   ├── system_info.h
│   └── terminal.h
├── src
│   ├── builtin.c
│   ├── builtin_table.c
│   ├── command.c
│   ├── dispatcher.c
│   ├── event.c
│   ├── executor.c
│   ├── job.c
│   ├── main.c
│   ├── parser.c
│   ├── shell.c
│   ├── shell_context.c
│   ├── sig.c
│   ├── system_info.c
│   └── terminal.c
└── tests
    ├── integration
    │   ├── test_integration_main.c
    │   ├── test_shell_background.c
    │   ├── test_shell_basic.c
    │   ├── test_shell_pipeline.c
    │   ├── test_shell_pty.c
    │   ├── test_shell_redirect.c
    │   ├── test_shell_runner.c
    │   └── test_shell_status.c
    ├── test_builtin_table.c
    ├── test_command.c
    ├── test_config.c
    ├── test_config.conf
    ├── test_dispatcher.c
    ├── test_event.c
    ├── test_executor.c
    ├── test_framework.c
    ├── test_framework.h
    ├── test_job.c
    ├── test_job_control.c
    ├── test_log.c
    ├── test_main.c
    ├── test_parser.c
    ├── test_shell_context.c
    └── test_system_info.c

```

## 技术栈

- C 语言(C11)
- Linux 应用/系统编程
- 进程管理机制
- fork/exec/wait等基础函数调用
- Pipe IPC通讯
- Signal 处理方式
- select 监听机制(后续升级若情况需要可能会引入epoll)
- Makefile
- Git
- GDB 调试 -->

## 后续方向

V1.6 开始，MiniShell 将逐步从纯PC/linux环境项目进入 ARM Linux 环境。

计划方向：

```
香橙派5plus
ARM Linux
UART
Device status
File/device event
Network event
Device command interface
```

## 当前项目roadmap

```
V1.2 Basic Stable                    OK
V1.3 Stability                       OK
V1.4 Unix Depth                      OK

V1.5 Engineering Release             <-this stage
 ├── Correctness / Build Baseline    OK
 ├── Test Framework                  OK
 ├── Unit Test Expansion             OK
 ├── Integration / System Test       OK
 ├── Resource Lifetime               OK
 ├── Job / Signal / Terminal         OK
 ├── Runtime Analysis                OK
 ├── Static Analysis                 OK
 ├── Documentation                   OK
 ├── CI                              OK
 ├── ARM-ready Build Interface       OK
 ├── CMake Build Parity
 └── Release Candidate Audit

V1.6 ARM / Orange Pi
V2.0 Embedded Device Terminal
```

