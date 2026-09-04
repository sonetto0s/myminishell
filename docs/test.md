# test文件说明

## 本文件用以记录测试指令运行结果,验证当前版本效果

## 当前版本 : V1.5

## 运行环境

- 系统:Ubuntu 22.04 / WSL Linux
- 编译方式:gcc(Makefile)
- 编程语言:C11
- 调试工具:GDB / ASan / UBSan / Valgrind / cppcheck


## 基础指令测试

```
pwd:
正常输出当前工作目录


echo hello:
hello


ls:
正常输出当前目录文件


不存在指令:
返回非0退出状态
Shell不会异常退出
```


## 重定向测试

```
echo hello > test.txt:
hello成功写入test文件


echo world >> test.txt:
world顺利追加至hello后


cat < test.txt:
hello
world
```


Builtin重定向:

```
pwd > test.txt
```

执行完成后Shell stdout正常恢复,后续Prompt不会继续写入文件。


Builtin重定向失败:

```
pwd > output.txt < not_exist_file
```

即使输入重定向失败,已经修改过的stdin/stdout仍会恢复。


External重定向失败:

```
cat < not_exist_file
echo alive
```

第一条命令失败后Shell仍可继续运行。


## Pipeline测试

```
echo hello | wc -c:
6


printf a\nb\nc\n | grep . | wc -l:
3
```


Pipeline退出状态:

```
sleep 1 | false
status
```

返回:

```
1
```

当前Pipeline退出状态使用最后一个Process的状态。


## 后台任务测试

```
sleep 10 &
```

Shell立即返回Prompt。


```
sleep 10 &
jobs
```

可正常输出Job状态。


多个后台任务:

```
sleep 10 &
sleep 20 &
jobs
```

JobManager可同时管理多个Job。


后台任务完成后:

```
SIGCHLD
 |
event_notify
 |
select
 |
job_reap
 |
job_cleanup_done
```

Child正常回收,不会留下Zombie。


## Ctrl+C测试

前台指令:

```
sleep 100
```

输入:

```
Ctrl+C
```

结果:

```
前台Process Group结束
Shell继续运行
Terminal恢复
Prompt重新出现
```


Prompt状态直接输入Ctrl+C:

```
>>MiniShell ^C
>>MiniShell
```

Shell不会退出。


## Ctrl+Z测试

```
sleep 20
```

输入:

```
Ctrl+Z
```

结果:

```
Job状态变为STOPPED
Terminal恢复给Shell
jobs可以查看停止任务
```


## fg测试

```
sleep 20
Ctrl+Z
fg
```

结果:

```
停止Job重新获得Terminal
SIGCONT继续运行
Shell等待Foreground Job
Job结束后Terminal重新返回Shell
```


## bg测试

```
sleep 20
Ctrl+Z
bg
```

结果:

```
停止Job重新运行
Job状态变为RUNNING
Shell仍可以继续输入命令
```


## Background TTY测试

```
cat &
```

后台cat尝试读取Terminal后:

```
SIGTTIN
```

Job进入STOPPED状态。

```
jobs
```

可以看到:

```
now it is stopped
```


## Ctrl+D测试

```
>>MiniShell
Ctrl+D
```

结果:

```
>>MiniShell 已退出
```


## sysinfo测试

```
sysinfo
```

可以读取:

```
Kernel
Hostname
Architecture
CPU Model
Memory Total
Memory Available
Uptime
```


## Unit Test

执行:

```
make test
```

当前结果:

```
Test Cases : 80
Assertions : 676
Passed     : 676
Failed     : 0
```


## Integration Test

执行:

```
make integration
```

当前结果:

```
Test Cases : 27
Assertions : 215
Passed     : 215
Failed     : 0
```


Integration Test目前包括:

```
基础命令
不存在命令
重定向
Builtin重定向恢复
Pipeline
Pipeline退出状态
后台任务
后台任务回收
Shell退出清理
PTY启动退出
Ctrl+C
Ctrl+Z
fg
bg
Pipeline Job Control
后台TTY停止
```


## 完整测试

执行:

```
make check
```

结果:

```
Unit Test PASS
Integration Test PASS
```


## Strict测试

执行:

```
make strict
```

编译参数:

```
-Wall
-Wextra
-Wpedantic
-Wformat=2
-Werror
```

当前结果:

```
0 warning
0 error
全部Unit/Integration Test通过
```


## ASan/LSan/UBSan测试

执行:

```
make asan
```

当前结果:

```
AddressSanitizer: PASS
LeakSanitizer: PASS
UndefinedBehaviorSanitizer: PASS

Unit Test: PASS
Integration Test: PASS
```


未发现:

```
heap-use-after-free
double-free
invalid memory access
memory leak
undefined behavior
```


## Valgrind测试

执行:

```
make valgrind
```

当前包含:

```
基础命令
重定向/Pipeline
后台任务退出
100 Child压力测试
```


当前结果:

```
FILE DESCRIPTORS: 3 open
in use at exit: 0 bytes in 0 blocks
All heap blocks were freed
ERROR SUMMARY: 0 errors
```


100 Child测试:

```
true &
true &
true &
...
100次
```

结果:

```
515 allocs
515 frees
0 leak
0 error
```


## cppcheck测试

执行:

```
make cppcheck
```

当前结果:

```
40/40 files checked
0 warning
0 error
```


## 完整静态测试

执行:

```
make static
```

包括:

```
make strict
make cppcheck
```

全部通过。


## 当前测试结论

```
普通功能测试通过
Unit Test通过
Integration Test通过
PTY Job Control测试通过
Signal/Event测试通过
ASan/LSan/UBSan通过
Valgrind通过
FD检查通过
100 Child压力测试通过
-Werror严格编译通过
cppcheck静态检查通过
```

目前V1.5核心代码在PC/Linux环境下已经完成较完整的测试与运行时检查。

