# architecture文件说明

## 本文件用以记录此项目整体架构实现,模块职责以及程序运行流程

## 整体架构

```

                         main
                          |
                     shell_init
                          |
                     log_init
                          |
                shell_context_init
                          |
                     event_init
                          |
                    signal_init
                          |
                   terminal_init
                          |
                     shell_run
                          |
                 select监听输入/Event
                          |
              |                           |
           stdin                       event
              |                           |
          getline                  event_drain
              |                           |
        trim_line                 signal_take_events
              |                           |
        parse_line             |                     |
              |             SIGCHLD                SIGINT
          tokenize               |                     |
              |              job_reap             清理输入状态
       build_command             |                 重新输出Prompt
              |
         Command填充
              |
    dispatcher_command
              |
        |                  |
     builtin            executor
        |                  |
   内建命令执行      execute_command
                           |
                  |                   |
            execute_single      execute_pipeline
                  |                   |
               fork()          pipe()/fork()
                  |                   |
               setpgid             setpgid
                  |                   |
            setredirect        pipe重定向处理
                  |                   |
       signal_reset_child    signal_reset_child
                  |                   |
               execvp()             execvp()
                  |                   |
             外部程序执行        Pipeline程序执行
                  |                   |
             Job/Process管理      Job/Process管理
                  |                   |
          job_wait_foreground   job_wait_foreground
                  |                   |
          terminal_restore      terminal_restore


                shell_cleanup
                     |
            shell_context_destroy
                     |
                job_shutdown
                     |
                job_destroy
                     |
                event_shut
                     |
                  Shell退出


后台任务流程:

 Command &
    |
execute_command
    |
fork/process group
    |
job_add
    |
process_add
    |
JobManager
    |
Shell继续运行
    |
SIGCHLD
    |
event_notify
    |
select监听Event
    |
event_drain
    |
signal_take_events
    |
job_reap
    |
更新Process状态
    |
更新Job状态
    |
job_cleanup_done


前台Job流程:

 Command
    |
fork/process group
    |
job_add/process_add
    |
terminal_set_foreground
    |
job_wait_foreground
    |
waitpid(-pgid)
    |
更新Process状态
    |
更新Job状态
    |
terminal_restore
    |
Shell重新获得终端


Ctrl+Z流程:

 Foreground Job
    |
Ctrl+Z
    |
SIGTSTP发送给前台Process Group
    |
Process停止
    |
waitpid返回WIFSTOPPED
    |
PROCESS_STOPPED
    |
JOB_STOPPED
    |
terminal_restore
    |
jobs可以查看状态


fg流程:

 stopped/running Job
    |
terminal_set_foreground
    |
如为STOPPED则job_continue
    |
SIGCONT
    |
job_wait_foreground
    |
Job结束/再次停止
    |
terminal_restore


bg流程:

 stopped Job
    |
job_continue
    |
SIGCONT
    |
PROCESS_RUNNING
    |
JOB_RUNNING
    |
Shell继续运行


Signal/Event流程:

 SIGCHLD/SIGINT
    |
signal handler
    |
设置pending_events
    |
event_notify
    |
self-pipe写入
    |
select唤醒
    |
event_drain
    |
signal_take_events
    |
Shell正常上下文处理事件


错误处理流程:

各模块返回错误码
 |
Error模块统一错误信息
 |
上层决定输出以及返回状态


资源释放流程:

Command创建
 |
parser填充argv/redirect
 |
dispatcher/executor使用
 |
command_free释放


Job创建
 |
job_add
 |
process_add
 |
Process运行/停止/结束
 |
job_reap更新状态
 |
job_remove/job_cleanup_done
 |
释放Job/Process


Shell退出
 |
job_shutdown
 |
结束RUNNING/STOPPED任务
 |
waitpid回收Child
 |
job_destroy
 |
event_shut

```


## V1.5 各层模块调用流程

```
主程序:
main
 |
shell_init      (初始化Shell整体运行环境)
 |
shell_run       (进入Shell主循环)
 |
shell_cleanup   (清理Shell资源并结束)



shell层(shell_run):
select          (同时监听stdin以及event fd)
 |
getline         (读取键盘输入指令)
 |
trim_line       (去除指令末尾换行符)
 |
parse_line      (解析输入)
 |
dispatcher_command   (判断输入指令类型)
 |
builtin/execute      (实现内部/外部指令执行)
 |
command_free         (释放当前Command数据)



parser层:
用户输入
 |
trim_line       (处理输入字符串)
 |
tokenize        (识别普通字符以及>|>>|<|&等符号)
 |
build_command   (将Token转化为Command结构)
 |
parse_line      (完成完整解析并返回Command链)



command层:
command_init     (初始化Command数据)
 |
Command使用
 |
command_free     (释放argv、redirect以及Command链)



dispatcher层:
dispatcher_command
 |
builtin_lookup       (查询builtin_table)
 |
builtin/executor     (内部指令直接执行,外部指令传入executor)



builtin层:
builtin_lookup
 |
cd/pwd/exit/help/jobs/status/sysinfo/fg/bg/reload
 |
执行对应Builtin函数



builtin_table层:
builtin_lookup        (按名称查询Builtin)
 |
builtin_get           (按序号获取Builtin信息)
 |
builtin_count         (获取Builtin数量)



executor层:
execute_command
 |
execute_single/execute_pipeline
 |
创建Child
 |
setpgid
 |
job_add/process_add
 |
setredirect
 |
signal_reset_child
 |
execvp


前台任务:
terminal_set_foreground
 |
job_wait_foreground
 |
terminal_restore


后台任务:
job_add/process_add
 |
直接返回Shell
 |
SIGCHLD异步处理



job层:
jobmanager_init       (初始化JobManager)
 |
job_add               (创建Job)
 |
process_add           (加入Process)
 |
job_wait_foreground   (等待前台Process Group)
 |
job_reap              (异步更新Child状态)
 |
job_cleanup_done      (清理已完成Job)
 |
job_remove            (删除指定Job)
 |
job_shutdown          (Shell退出时处理剩余任务)
 |
job_destroy           (释放全部Job/Process数据)


Process状态:
PROCESS_RUNNING
 |
PROCESS_STOPPED
 |
PROCESS_DONE


Job状态:
JOB_RUNNING
 |
JOB_STOPPED
 |
JOB_DONE


Job状态根据内部Process状态统一计算



signal层:
signal_init
 |
注册SIGCHLD/SIGINT handler
 |
忽略SIGQUIT/SIGTSTP/SIGTTIN/SIGTTOU
 |
Signal到达
 |
pending_events更新
 |
event_notify


子进程执行前:
signal_reset_child
 |
恢复SIGINT/SIGQUIT/SIGTSTP/SIGTTIN/SIGTTOU/SIGCHLD默认行为
 |
清空signal mask
 |
execvp



event层:
event_init
 |
创建self-pipe
 |
设置O_NONBLOCK
 |
设置FD_CLOEXEC
 |
event_getfd
 |
select监听
 |
event_notify
 |
event_drain
 |
event_shut


Child执行前:
event_close_in_child
 |
关闭Shell内部Event FD



terminal层:
terminal_init
 |
确认是否为TTY
 |
记录Shell pgid
 |
terminal_set_foreground
 |
tcsetpgrp切换前台Process Group
 |
terminal_restore
 |
恢复Shell pgid



shell_context层:
shell_context_init
 |
running初始化
 |
JobManager初始化
 |
config初始化与加载
 |
last_exit_status初始化


退出时:
shell_context_destroy
 |
job_shutdown
 |
job_destroy
 |
running=0



config配置层:
config_init
 |
设置默认配置
 |
config_load
 |
读取config/config.conf
 |
prompts/max_job/debug


reload:
builtin_reload
 |
config_load
 |
更新ShellContext中的config



log层:
log_init
 |
log_write
 |
log_debug/log_info/log_error
 |
输出DEBUG/INFO/ERROR日志



error模块层:
MiniShellError
 |
minishell_error_string
 |
统一错误信息



system_info层:
system_info_collect
 |
读取Kernel/Hostname/Architecture/CPU/Memory/Uptime
 |
builtin_sysinfo
 |
输出系统状态



测试层:
test_framework
 |
Unit Test
 |
80 Test Cases
 |
676 Assertions


Integration Test
 |
真实启动MiniShell
 |
basic/redirect/pipeline/status/background
 |
PTY Job Control
 |
27 Test Cases
 |
215 Assertions


运行时分析:
make asan
 |
ASan/LSan/UBSan


make valgrind
 |
内存检查
 |
FD检查
 |
Background shutdown
 |
100 Child stress


静态分析:
make strict
 |
-Werror


make cppcheck
 |
40个源码/测试文件静态检查


make static
 |
strict + cppcheck

```
