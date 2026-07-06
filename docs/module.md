# module文件说明

## 当前架构所需函数及其作用说明

- shell.c：控制变量生命周期,被main文件调用
- parser.c 设计分离解析指令函数,传导入结构体中供后续调用
- utils.c：工具函数,为shell文件辅助调用
- executor.c 控制进程实现
- main.c：程序入口