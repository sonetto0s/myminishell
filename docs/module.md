# module文件说明

## 当前架构所需函数及其作用说明


- main.c:  程序入口,负责启动MiniShell

- shell.c: 控制Shell整体运行流程,负责初始化and运行以及资源释放

- shell_context.c: 管理Shell运行状态,统一保存Shell相关数据

- parser.c: 负责命令解析,将用户输入分割并写入Command结构体

- command.c:  负责Command结构体创建初始化以及释放

- dispatcher.c: 负责判断命令类型,分发内部命令和外部命令

- executor.c: 负责外部命令执行,实现进程创建,管道以及重定向功能

- builtin.c:  负责Shell内部命令实现,如cd、pwd、exit等基础指令

- sig.c:  负责信号处,实现Ctrl+C以及SIGCHLD相关功能

- job.c:  负责后台任务管理,记录和回收后台运行进程

- event.c: 负责事件驱动机制，通过select监听

- utils.c: 提供公共工具函数,辅助其他模块

- log.c: 负责日志系统,实现DEBUG、INFO、ERROR等日志输出

- error.c: 负责统一错误处理,管理错误信息

- config.c: 负责配置管理,实现配置初始化以及配置文件读取

## 测试文件

- test_main.c: 测试程序入口

- test_config.c:测试config模块功能

- test_parser.c: 测试parser模块功能

- test_log.c: 测试log模块功能