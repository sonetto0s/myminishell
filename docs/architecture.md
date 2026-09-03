# architecture文件说明

## 本文件用以记录此项目整体架构实现,模块职责以及程序运行流程

## 整体架构
```

                         main
                          |
                     shell_init
                          |
                 初始化ShellContext
                          |
        |                 |                |
     config              log             signal
        |                 |                |
 config_load          log_init        signal_init
        |                 |                |
                          |
                     初始化Event
                          |
                     shell_run
                          |
                     读取用户输入
                          |
                       fgets
                          |
                     trim_line
                          |
                    parse_line
                          |
                      tokenize
                          |
                   build_command
                          |
                    Command填充
                          |
                 dispatcher_command
                          |
              |                       |
           builtin                executor
              |                       |
              |                       |
          内建命令执行          execute_command
                                      |
                         |                         |
                 execute_single          execute_pipeline
                         |                         |
                                      |
                               setredirect()
                                      |
                               run_process()
                                      |
                                    fork()
                                      |
                                   execvp()
                                      |
                                 外部程序执行


                shell_cleanup()
                     |
                event_shut()
                     |
                 Shell退出

后台任务流程:

 Command
    |
job_add
    |
JobManager
    |
select监听Event
    |
event_notify
    |
sigchld_handler
    |
job_reap()


错误处理流程:

各模块返回错误码
 |
Error模块统一错误处理


```


## V1.4  各层模块调用流程
```
主程序:
main
 |
shell_init      (初始化)
 |
shell_run       (调用shell层)
 |
shell_cleanup   (清理数据并结束)



shell层(shell_run):
Command com     (定义并初始化com结构体用以存放指令)
 |
fgets           (读取键盘输入指令)
 |
trim_line       (去除指令末尾的'\n',转化为'\0')
 |
parse_line      (实现字符串的分离解析,转化为字符后存入com结构体)
 |
dispatcher_command   (判断输入指令类型)
 |
built/execute        (实现内部/外部指令执行)
 |


parser层:
fgets           (读取输入)
 |
trim_line       (分割字符串)
 |
tokenize        (引入tokenize,分析复杂指令符号,优化输入字符解析)
 |
build_command   (判断复杂指令符号,将结构体填充,传导至parse_line函数)
 |
parse_line      (实现tokenize和build_command函数调用)


dispatcher层:
dispatcher_command   (实现返回判断指令类型)
 |
builtin/executor     (解析判断内部/外部指令并执行)


executor 层:
execute_command                     (判断命令参数类型)
 |
execute_single/execute_pipeline     (实现有无管道命令执行)
 |
setredirect                         (实现重定向)
 |
run_process                         (实现进程转换)


signal 层:
signal_init/signal_reset_child      (实现信号数据初始化+实现Ctrl+C)
 |
sigchld_handler/sigint_handler      (优化信号调用逻辑)

shell_context 层:
shell_status_init                   (初始化shell状态结构体)


event 层:
event_init                          (初始化事件驱动)
 |
event_getfd                         (返回读端值)
 |
event_notify                        (反馈事件驱动状态)
 |
event_shut                          (关闭文件)


shell_context 层:
shell_context_init                  (初始化shell状态管理)


job 层:
job_init                             (初始化job状态管理)
    |
job_add                              (填充job数据)
    |
JobManager                           (管理job自身顺序)
    |
job_reap                             (回收job数据中pid部分)

config 配置层:
config_init                          (初始化config配置)
      |
config_load                          (装载config.conf配置)
      |
config.conf                          (提供配置)

log 层:
log_init                             (还能是啥，实在懒得写了☠️)
 |
log_write                            (提供类printf模板)
 |
log_info/error/debug                 (打印调试信息)

error 模块层:
minishell_error_string                (提供errro报错类型)

```
