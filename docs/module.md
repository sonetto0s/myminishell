# module文件说明

## 当前架构所需函数及其作用说明

- shell.c：控制变量生命周期,被main文件调用
- parser.c 设计分离解析指令函数,传导入结构体中供后续调用
- utils.c：工具函数,为shell文件辅助调用
- dispatcher.c 实现内部外部指令类型判断
- executor.c 控制外部指令输入下进程实现
- builtin.c  控制内部指令输入下的进程实现
- main.c：程序入口