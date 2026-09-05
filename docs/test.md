# test文件说明

## 本文件用以记录测试指令运行结果,验证当前版本效果

## 当前版本 : V1.5

## 运行环境

- 系统:Ubuntu 22.04 / WSL Linux
- 编译方式:GCC / GNU Make / CMake
- 编程语言:C11
- 调试工具:GDB / ASan / LSan / UBSan / Valgrind / cppcheck

## 基础指令测试

```
pwd:
正常输出当前工作目录


echo hello:
hello


ls:
正常输出当前目录文件


不存在指令:
返回127
Shell不会异常退出


存在但无法执行的目标:
返回126
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

执行完成后Shell stdout正常恢复,后续Prompt不会继续写入文件.

Builtin重定向失败:

```
pwd > output.txt < not_exist_file
```

即使输入重定向失败,已经修改过的stdin/stdout仍会恢复.

External重定向失败:

```
cat < not_exist_file
echo alive
```

第一条命令失败后Shell仍可继续运行.

Builtin输出设备失败:

```
pwd > /dev/full
```

当前要求:

```
Builtin输出flush失败
命令返回1
stdout恢复
后续命令仍可正常执行
```

## Pipeline测试

```
echo hello | wc -c:
6


seq 3 | grep 2 | wc -l:
1
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

当前Pipeline退出状态使用最后一个Process的状态,不实现pipefail.

## 后台任务测试

```
sleep 10 &
```

Shell立即返回Prompt.

```
sleep 10 &
jobs
```

可正常输出Job状态.

多个后台任务:

```
sleep 10 &
sleep 20 &
jobs
```

JobManager可同时管理多个Job.

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

Child正常回收,不会留下Zombie.

## Input生命周期测试

V1.5新增分段输入回归:

```
sleep 0.2 &
echo PART
```

这里`echo PART`暂时不发送换行,等待后台`sleep`结束产生SIGCHLD.

要求:

```
SIGCHLD出现后不能提前执行PART
继续补入IAL\n后
只执行PARTIAL
```

当前Integration Test已经覆盖这个场景.

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
退出状态为128+SIGINT
Shell继续运行
Terminal恢复
Prompt重新出现
```

Prompt状态直接输入Ctrl+C:

```
>>MiniShell ^C
>>MiniShell
```

当前未完成输入会被丢弃,Shell不会退出.

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
保存Job Terminal Modes
Terminal恢复给Shell
Shell termios恢复
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
恢复Job Terminal Modes
SIGCONT继续运行
Shell等待Foreground Job
Job结束或再次停止后Terminal重新返回Shell
```

额外覆盖:

```
fg < /dev/null
```

Terminal控制使用独立`/dev/tty`,不会因为fd 0被重定向导致Foreground切换失败.

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

Job进入STOPPED状态.

Shell自身如果在父Shell后台启动,也会遵循Foreground Process Group规则,不会主动抢走父Shell的Terminal.

## FIFO Signal测试

Integration PTY测试包含:

```
cat < fifo
Ctrl+C
```

要求阻塞在FIFO open阶段的Child仍使用正确的External Signal语义并结束.

同时包含:

```
cat < fifo
Ctrl+Z
jobs
fg
Ctrl+C
```

用于验证Child startup signal、STOPPED状态、fg以及Terminal交接完整流程.

## Termios测试

测试程序会:

```
stty -echo -icanon
停止自身
```

要求:

```
Job停止后Shell恢复ECHO/ICANON
fg后Job恢复停止前Terminal Modes
Job结束后Shell再次恢复自身Terminal Modes
```

## Job Shutdown测试

Unit Test覆盖:

```
Running Process shutdown
Stopped Process shutdown
Multiple Process shutdown
Same Process Group descendant shutdown
```

其中同组后代测试会创建一个忽略SIGTERM的后代Process.

要求:

```
TERM整个pgid
有限等待
KILL整个pgid
Direct Child全部waitpid回收
同组后代不能残留
```

## Signal/Event Shutdown测试

测试Shell内部Signal/Event关闭顺序:

```
signal_shutdown
 |
event_shut
```

关闭后再次触发SIGINT不能因为self-pipe已经关闭而使进程被SIGPIPE终止.

## FD_SETSIZE测试

测试进程预先打开大量FD,让MiniShell创建的Event FD超过`FD_SETSIZE`.

要求:

```
Shell在FD_SET前检测范围
输出明确错误
干净返回1
不能触发FD_SET越界
```

## Config测试

Config Unit Test包含:

```
默认配置
合法单行解析
非法max_job
文件加载
缺失文件
目录读取失败
事务式加载失败
```

事务式加载测试要求:

```
新配置前几行合法
中间出现非法配置
 |
config_load返回失败
 |
旧MiniShellConfig完整保留
```

Integration同时覆盖:

```
cd /
reload
```

`reload`仍然使用Shell启动时确定的配置来源.

## Ctrl+D测试

```
>>MiniShell
Ctrl+D
```

结果:

```
>>MiniShell 已退出
```

如果EOF前还有一条没有换行但已经完整输入的数据,Shell会先处理最后一行再退出.

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

同时测试重复collect覆盖已有SystemInfo结构体.

## Unit Test

执行:

```bash
make test
```

当前结果:

```
Test Cases : 90
Assertions : 726
Passed     : 726
Failed     : 0
```

## Integration Test

执行:

```bash
make integration
```

当前结果:

```
Test Cases : 44
Assertions : 348
Passed     : 348
Failed     : 0
```

Integration Test当前包括:

```
基础命令
不存在命令
126/127状态
输入/输出/追加重定向
Builtin重定向恢复
Builtin /dev/full输出失败
Builtin输出失败后恢复
Pipeline
Pipeline退出状态
后台任务
后台任务回收
Shell退出清理
稳定Config reload
高FD边界
分段输入 + SIGCHLD
PTY启动退出
Ctrl+C
Ctrl+Z
fg
bg
Pipeline Job Control
后台TTY停止
fg < /dev/null
FIFO Ctrl+C
FIFO Ctrl+Z
Job/Shell termios恢复
后台启动Terminal规则
```

## 当前测试总量

```
Unit Test:
90 Cases
726 Assertions

Integration Test:
44 Cases
348 Assertions

Total:
134 Cases
1074 Assertions
0 Failed
```

## 完整测试

执行:

```bash
make check
```

结果要求:

```
Unit Test PASS
Integration Test PASS
```

## Strict测试

执行:

```bash
make strict
```

编译参数:

```
-Wall
-Wextra
-Wpedantic
-Wformat=2
-Wstrict-prototypes
-Werror
```

当前快照验证结果:

```
Unit Test: 90 / 726 / 0 Failed
Integration Test: 44 / 348 / 0 Failed
0 warning
0 error
```

## ASan/LSan/UBSan测试

执行:

```bash
make asan
```

当前快照验证结果:

```
AddressSanitizer: PASS
LeakSanitizer: PASS
UndefinedBehaviorSanitizer: PASS

Unit Test: 90 / 726 / 0 Failed
Integration Test: 44 / 348 / 0 Failed
```

当前未发现:

```
heap-use-after-free
double-free
invalid memory access
memory leak
undefined behavior
```

## Valgrind测试

执行:

```bash
make valgrind
```

当前Makefile包含:

```
基础命令
重定向/Pipeline
后台任务退出
100 Child压力测试
```

此前阶段已经完成Valgrind零泄漏检查,但V1.5最终tag前仍应使用当前最终源码重新执行一次,最终记录以最后一次Final Gate输出为准.

## cppcheck测试

执行:

```bash
make cppcheck
```

最终V1.5 tag前需要使用当前最终源码重新确认:

```
0 warning
0 error
```

## 完整静态测试

执行:

```bash
make static
```

包括:

```
make strict
make cppcheck
```

## CMake测试

Debug:

```bash
cmake -S . -B /tmp/minishell-cmake-debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DMINISHELL_WARNINGS_AS_ERRORS=ON
cmake --build /tmp/minishell-cmake-debug --parallel
ctest --test-dir /tmp/minishell-cmake-debug --output-on-failure
```

Release:

```bash
cmake -S . -B /tmp/minishell-cmake-release \
    -DCMAKE_BUILD_TYPE=Release \
    -DMINISHELL_WARNINGS_AS_ERRORS=ON
cmake --build /tmp/minishell-cmake-release --parallel
ctest --test-dir /tmp/minishell-cmake-release --output-on-failure
```

V1.5要求Debug/Release都可以在源码树外独立构建并运行CTest,测试不能依赖源码目录中的`build/`路径.

## ARM64测试

执行:

```bash
make arm64
file build/arm64/minishell
make arm64-package
```

需要确认:

```
ARM aarch64

dist/arm64/bin/minishell
dist/arm64/config/config.conf
```

V1.5只验证ARM64构建以及Package接口,真实Orange Pi Runtime验证进入V1.6.

## Final Gate

V1.5正式tag前最终执行:

```
make clean && make strict
make clean && make asan
make cppcheck
make clean && make valgrind
CMake Debug out-of-source
CMake Release out-of-source
make package
make arm64
make arm64-package
git diff --check
GitHub Actions全部通过
```



## 当前测试结论

```
普通功能测试通过
Unit Test通过
Integration Test通过
Input生命周期测试通过
PTY Job Control测试通过
Signal/Event测试通过
Terminal/termios测试通过
Config事务测试通过
高FD边界测试通过
ASan/LSan/UBSan通过
-Werror严格编译通过
```
