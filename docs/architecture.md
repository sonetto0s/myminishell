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
                   terminal_init
                          |
                    signal_init
                          |
                     shell_run
                          |
                select监听stdin/Event
                          |
              |                           |
           stdin                       event
              |                           |
        read 1 byte                 event_drain
              |                           |
       input_buffer              signal_take_events
              |                           |
       遇到换行?              |                     |
          |                 SIGCHLD                SIGINT
        parse_line              |                     |
          |                  job_reap            清理输入状态
       tokenize                 |                 重新输出Prompt
          |
    build_command
          |
    Command填充
          |
 dispatcher_command
          |
     |                  |
  builtin            executor
     |                  |
内建命令执行       execute_command
                        |
               |                   |
         execute_single      execute_pipeline
               |                   |
            fork()          pipe()/fork()
               |                   |
          startup gate        startup gate
               |                   |
          setpgid/Job         setpgid/Job
               |                   |
       terminal handoff    terminal handoff
               |                   |
           release GO          release GO
               |                   |
       signal_reset_child  signal_reset_child
               |                   |
          setredirect        pipe/redirect
               |                   |
            execvp()             execvp()
               |                   |
          外部程序执行       Pipeline程序执行
               |                   |
       Job/Process管理      Job/Process管理
               |                   |
       job_wait_foreground  job_wait_foreground
               |                   |
        保存Job termios      保存Job termios
               |                   |
        terminal_restore      terminal_restore


                shell_cleanup
                     |
            shell_context_destroy
                     |
                job_shutdown
                     |
               signal_shutdown
                     |
                event_shut
                     |
             terminal_shutdown
                     |
                  Shell退出
```

## V1.5 各层模块调用流程

```
输入层
 |
shell.c
 |
ShellContext input_buffer
 |
parser.c
 |
Command / Command链
 |
dispatcher.c
 |
 |----------------------|
 |                      |
builtin.c            executor.c
 |                      |
父进程内执行          Child/Process Group
 |                      |
重定向保护             startup gate
 |                      |
状态返回                fork/exec/pipe
 |                      |
 |                  Job/Process
 |                      |
 |                    job.c
 |                      |
 |                  terminal.c
 |                      |
 |----------------------|
          |
   ShellContext状态
```

## 输入流程

V1.5不再使用一次getline调用代表一整条命令.

```
select
 |
stdin readable
 |
read 1 byte
 |
写入ctx->input_buffer
 |
 |--------------------|
 |                    |
不是换行              遇到换行
 |                    |
继续select         shell_dispatch_input
                       |
                   parse_line
                       |
                  dispatcher
```

这样做主要解决两个问题:

```
SIGCHLD等事件打断输入
不会把半条命令提前执行

Shell不会一次预读后续stdin数据
避免吃掉Foreground Child应该读取的数据
```

当输入超过`SHELL_INPUT_SIZE`时,Shell进入discard状态,丢弃当前超长行直到下一次换行,并把状态设置为语法错误.

Ctrl+C发生在Prompt输入阶段时:

```
SIGINT
 |
Event唤醒
 |
shell_reset_input
 |
丢弃当前未完成输入
 |
重新输出Prompt
```

## Child启动流程

单命令与Pipeline都使用startup gate.

```
fork
 |
 |--------------------------------|
 |                                |
Child                           Parent
 |                                |
block startup signals            setpgid
 |                                |
setpgid                          job_add
 |                                |
close Event FD                  process_add
 |                                |
wait startup GO           terminal_set_foreground
 |                                |
 |<------------- GO --------------|
 |
signal_reset_child
 |
redirect
 |
execvp
```

这个流程保证Foreground Child不会在Parent完成Process Group、Job登记以及Terminal交接前进入用户程序.

Child在等待startup gate期间暂时Block交互相关Signal,收到GO后恢复默认Signal disposition以及Signal mask,然后才进入redirect/exec阶段.

如果startup流程中Parent端发生Job/Process登记失败等问题,未正式发布的Child会进入rollback路径并使用SIGKILL确定性清理,避免STOPPED Child导致无界wait.

## Pipeline流程

```
Command链
 |
逐个创建pipe/fork
 |
所有Child进入同一pgid
 |
Child等待startup gate
 |
Parent完成Job/Process登记
 |
Foreground时完成Terminal交接
 |
一次释放所有Child
 |
Child建立pipe stdin/stdout
 |
处理本命令redirect
 |
execvp
 |
Parent等待整个Process Group
```

Pipeline退出状态继续使用最后一个Process的状态,当前不实现pipefail.

## 后台任务流程

```
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
startup gate release
 |
Shell立即返回Prompt
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
更新Process/Job状态
 |
job_cleanup_done
```

## 前台Job流程

```
Command
 |
fork/process group
 |
startup gate
 |
job_add/process_add
 |
terminal_set_foreground
 |
release child
 |
job_wait_foreground
 |
waitpid(-pgid)
 |
更新Process状态
 |
更新Job状态
 |
如果STOPPED则保存Job termios
 |
terminal_restore
 |
Shell重新获得Terminal以及Shell termios
```

## Ctrl+Z流程

```
Foreground Job
 |
Ctrl+Z
 |
SIGTSTP发送给Foreground Process Group
 |
Process停止
 |
waitpid返回WIFSTOPPED
 |
PROCESS_STOPPED
 |
JOB_STOPPED
 |
保存Job Terminal Modes
 |
terminal_restore
 |
jobs可以查看状态
```

## fg流程

```
STOPPED/RUNNING Job
 |
terminal_set_foreground
 |
恢复Job Terminal Modes
 |
如为STOPPED则job_continue
 |
SIGCONT
 |
job_wait_foreground
 |
Job结束/再次停止
 |
保存新的Job Terminal Modes
 |
terminal_restore
```

控制TTY由terminal模块单独打开`/dev/tty`,不依赖STDIN_FILENO,因此fd 0被Builtin重定向后仍然可以进行Foreground Process Group切换.

## bg流程

```
STOPPED Job
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
```

## Signal/Event流程

```
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
```

Signal Handler只完成轻量事件记录以及self-pipe通知,不会直接调用waitpid、malloc、printf等复杂逻辑.

Shell退出时:

```
job_shutdown
 |
结束/回收现有Job
 |
signal_shutdown
 |
关闭Signal通知路径
 |
event_shut
 |
关闭self-pipe
 |
terminal_shutdown
```

这样可以避免Handler仍在使用Event pipe时另一边已经关闭FD的竞态.

## Job Shutdown流程

```
Shell退出
 |
STOPPED Job先SIGCONT
 |
对Job pgid发送SIGTERM
 |
有限次数尝试回收
 |
仍存在的Process Group发送SIGKILL
 |
waitpid回收MiniShell直接Child
 |
job_destroy
```

Signal范围以Process Group为单位,waitpid只负责MiniShell实际拥有的直接Child.

即使登记的直接Child已经结束,最终仍会尝试对原pgid进行SIGKILL,用于清理仍留在同组内且忽略SIGTERM的后代进程.

## Builtin重定向流程

```
Builtin Command
 |
保存原stdin/stdout
 |
打开重定向文件
 |
dup2切换标准流
 |
执行Builtin
 |
fflush检查真实输出结果
 |
恢复原stdin/stdout
 |
clearerr
 |
返回命令状态
```

如果Builtin逻辑本身成功,但最终输出flush失败,命令仍返回非0状态.

如果Shell无法恢复自己的标准流,视为Shell健康状态失败,不会继续假装正常运行.

## 外部命令退出状态

当前V1.5采用:

```
0        成功
1        普通/内部/重定向失败
2        Parser语法错误
126      找到目标但无法执行
127      命令未找到
128+sig  Signal结束
```

Pipeline使用最后一个Process的退出状态.

## Config流程

```
Shell启动
 |
记录启动工作目录
 |
生成稳定config_file路径
 |
config_load
 |
加载temporary config
 |
完整parse/validate
 |
 |----------------|
 |                |
成功             失败
 |                |
commit         保留旧配置
```

启动后执行`cd`不会改变`reload`使用的配置来源.

V1.5只确定启动时的稳定配置身份,正式ARM Linux安装路径留到V1.6根据真实板端环境决定.

## Error处理流程

```
发生错误的底层模块
 |
掌握errno/path/pid等上下文
 |
输出具体诊断
 |
返回MiniShell错误码/Unix状态
 |
上层负责状态归一以及生命周期决策
```

避免上层在已经有具体错误信息后再重复输出无意义的`unknown error`.

## 资源释放流程

```
Command创建
 |
parser填充argv/redirect
 |
dispatcher/executor使用
 |
command_free释放
```

```
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
```

```
Shell退出
 |
job_shutdown
 |
结束Process Group
 |
回收Direct Child
 |
job_destroy
 |
signal_shutdown
 |
event_shut
 |
terminal_shutdown
```
