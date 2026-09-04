# module文件说明

## 当前架构所需函数及其作用说明

- main.c: 程序入口,负责启动MiniShell并管理Shell生命周期

- shell.c: 控制Shell整体运行流程,负责初始化、select主循环、Signal/Event处理以及资源释放

- shell_context.c: 管理Shell运行状态,统一保存Shell相关数据,包括JobManager、配置以及last_exit_status

- parser.c: 负责命令解析,将用户输入Tokenize后写入Command结构体以及Command链

- command.c: 负责Command结构体初始化以及Command/argv/redirect相关资源释放

- dispatcher.c: 负责判断命令类型,查询Builtin Table,分发内部命令和外部命令

- executor.c: 负责外部命令执行,实现fork、execvp、Process Group、管道、重定向以及前台任务等待

- builtin.c: 负责Shell内部命令实现,包括cd、pwd、exit、help、jobs、status、sysinfo、fg、bg、reload等指令

- builtin_table.c: 负责Builtin指令集中注册以及查询,避免dispatcher内部大量if/else判断

- sig.c: 负责Signal初始化以及事件记录,实现SIGCHLD、Ctrl+C以及Child执行前Signal恢复

- job.c: 负责Job/Process状态管理,实现Job创建、Process加入、前后台等待、状态更新、回收以及Shell退出清理

- event.c: 负责self-pipe事件通知机制,将Signal事件传递至select主循环处理

- terminal.c: 负责TTY以及Foreground Process Group控制,实现Shell与Job之间终端所有权切换

- utils.c: 提供公共工具函数,辅助其他模块

- log.c: 负责日志系统,实现DEBUG、INFO、ERROR等日志输出

- error.c: 负责统一错误处理,管理MiniShell错误信息

- config.c: 负责配置管理,实现配置初始化、配置文件读取以及reload支持

- system_info.c: 负责设备/系统状态识别,读取Kernel、CPU、Memory、Architecture、Uptime等信息


## 主要数据结构

- ShellContext: 统一管理Shell运行状态

```
running
JobManager
last_exit_status
MiniShellConfig
config_file
```

- Command: 保存解析完成后的单条命令

```
argc
argv
background
redirect
next
```

- Command链: 用于表示Pipeline

```
Command
 |
Command
 |
Command
```

- JobManager: 统一管理当前Shell中的Job

```
head
nextid
```

- Job: 表示一个完整任务或者Pipeline

```
id
pgid
status
command
processes
next
```

- Process: 表示Job内部的单个Child

```
pid
status
wait_status
wait_status_valid
next
```

- MiniShellConfig: 保存当前Shell配置

```
prompts
max_job
debug
```


## Job状态

- JOB_RUNNING: Job中存在正在运行的Process

- JOB_STOPPED: Job中没有运行Process,但存在停止Process

- JOB_DONE: Job中所有Process均已结束


## Process状态

- PROCESS_RUNNING: Process正在运行

- PROCESS_STOPPED: Process被停止

- PROCESS_DONE: Process已经结束并完成状态记录


## Signal/Event相关函数

- signal_init: 初始化Shell Signal处理策略

- signal_take_events: 读取并清除当前pending Signal事件

- signal_reset_child: Child执行execvp前恢复默认Signal行为

- event_init: 创建self-pipe并初始化Event机制

- event_getfd: 获取self-pipe读端

- event_notify: 向Event pipe写入事件通知

- event_drain: 清空Event pipe已有数据

- event_close_in_child: Child关闭Shell内部Event FD

- event_shut: Shell退出时关闭Event FD


## Job相关函数

- jobmanager_init: 初始化JobManager

- job_add: 新建Job并加入JobManager

- process_add: 向Job加入Process

- job_find: 根据pgid查找Job

- job_list: 输出当前Job状态

- job_wait_foreground: 等待整个Foreground Process Group状态变化

- job_continue: 向停止Job发送SIGCONT并更新状态

- job_exit_status: 获取Job最后一个Process退出状态

- job_count_active: 统计当前活动Job数量

- job_reap: 使用waitpid异步回收Child状态

- job_cleanup_done: 清理已经完成的Job

- job_remove: 删除指定Job

- job_shutdown: Shell退出时结束并回收剩余Job

- job_destroy: 释放JobManager中的全部Job/Process数据


## Terminal相关函数

- terminal_init: 初始化Terminal状态并记录Shell pgid

- terminal_is_initialized: 判断当前是否拥有可用TTY环境

- terminal_set_foreground: 将指定Process Group设置为Terminal前台

- terminal_restore: 将Terminal前台重新恢复至Shell


## 测试文件

- test_main.c: Unit Test程序入口

- test_framework.c/test_framework.h: 自定义轻量测试框架

- test_config.c: 测试config模块功能

- test_parser.c: 测试parser模块功能

- test_log.c: 测试log模块功能

- test_event.c: 测试Event生命周期以及FD行为

- test_shell_context.c: 测试ShellContext初始化以及销毁

- test_builtin_table.c: 测试Builtin Table查询功能

- test_system_info.c: 测试System Info读取

- test_dispatcher.c: 测试Builtin/External命令分发

- test_executor.c: 测试单命令、Pipeline以及Child回收

- test_job.c: 测试JobManager、Job、Process以及Job状态变化

- test_job_control.c: 测试Signal/Event/Foreground Job Control相关逻辑

- test_command.c: 测试Command初始化以及资源释放


## Integration测试文件

- test_integration_main.c: Integration Test入口

- test_shell_runner.c: 启动真实MiniShell并输入测试脚本

- test_shell_basic.c: 测试基础命令

- test_shell_redirect.c: 测试输入/输出/追加重定向

- test_shell_pipeline.c: 测试多级Pipeline以及返回状态

- test_shell_status.c: 测试Shell退出状态

- test_shell_background.c: 测试后台任务以及SIGCHLD回收

- test_shell_pty.c: 使用PTY测试Ctrl+C、Ctrl+Z、fg、bg以及Terminal Job Control


## 当前测试状态

```
Unit Test:

80 Cases
676 Assertions
0 Failed


Integration Test:

27 Cases
215 Assertions
0 Failed
```

