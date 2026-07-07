# myminishell

## 项目简介
 这是一个基于 Linux 的MiniShell,用来实现进程控制与命令解析。

## 当前版本: V0.3

## 更新日志
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

## 技术节点
```
V0.3:
dispatcher架构整体构建
指令解析正式区分为builtin与executor两部分
V0.2.1:
tokenize模块编写
execvp引入
进一步提升指令解析能力
V0.2:
strtok函数调用
实现初步分解指令功能
V0.1:
Linux进程模型构建
fgets输入与strncmp函数解析指令
工程模块化设计
```
## 运行方式

- Makefile
- make
- ./minishell

## 项目结构

```
.
├── build
├── common
├── docs
│   ├── architecture.md
│   ├── debug_log.md
│   └── module.md
├── include
│   ├── builtin.h
│   ├── dispatcher.h
│   ├── executor.h
│   ├── log.h
│   ├── parser.h
│   ├── shell.h
│   └── utils.h
├── Makefile
├── README.md
└── src
    ├── builtin.c
    ├── dispatcher.c
    ├── executor.c
    ├── log.c
    ├── main.c
    ├── parser.c
    ├── shell.c
    └── utils.c

```