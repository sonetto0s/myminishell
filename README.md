# myminishell

## 项目简介
 这是一个基于 Linux 的MiniShell,用来实现进程控制与命令解析。

## 当前版本: V0.9.3

## 更新日志

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


## 运行方式

- Makefile
- make
- ./shell

## 项目结构

```
.
├── build
│   ├── common
│   │   ├── log.o
│   │   └── utils.o
│   └── src
│       ├── builtin.o
│       ├── command.o
│       ├── dispatcher.o
│       ├── executor.o
│       ├── main.o
│       ├── parser.o
│       ├── shell.o
│       └── sig.o
├── common
│   ├── log.c
│   ├── log.h
│   ├── utils.c
│   └── utils.h
├── docs
│   ├── architecture.md
│   ├── debug_log.md
│   └── module.md
├── include
│   ├── builtin.h
│   ├── command.h
│   ├── dispatcher.h
│   ├── executor.h
│   ├── parser.h
│   ├── shell_context.h
│   ├── shell.h
│   └── sig.h
├── Makefile
├── README.md
├── shell
└── src
    ├── builtin.c
    ├── command.c
    ├── dispatcher.c
    ├── executor.c
    ├── main.c
    ├── parser.c
    ├── shell.c
    ├── shell_context.c
    └── sig.c

```