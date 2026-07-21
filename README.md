# myminishell

## 项目简介
 这是一个基于 Linux 的MiniShell,用来实现进程控制与命令解析。

## 当前版本: V0.5.1

## 更新日志

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
│       └── shell.o
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
│   └── shell.h
├── Makefile
├── README.md
├── shell
├── src
│   ├── builtin.c
│   ├── command.c
│   ├── dispatcher.c
│   ├── executor.c
│   ├── main.c
│   ├── parser.c
│   └── shell.c


```