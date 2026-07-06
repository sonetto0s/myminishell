# myminishell

## 项目简介
 这是一个基于 Linux 的MiniShell,用来实现进程控制与命令解析。

## 当前版本: V0.1

## 更新日志

- V0.1：初始化项目,初步构建Shell生命周期框架

## 技术节点
```
V0.1:
Linux进程模型构建
fgets输入与strncmp函数解析指令
工程模块化设计
```
## 运行方式

Makefile
make
./MiniShell

## 项目结构

```
├── build
├── common
├── docs
│   ├── architecture.md
│   ├── debug_log.md
│   └── module.md
├── include
│   ├── log.h
│   ├── parser.h
│   ├── shell.h
│   └── utils.h
├── Makefile
├── README.md
└── src
    ├── log.c
    ├── main.c
    ├── parser.c
    ├── shell.c
    └── utils.c
```