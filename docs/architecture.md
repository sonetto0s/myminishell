# architecture文件说明

## 本文件用以记录此项目整体架构实现,整体运行逻辑

## V0.5.3 架构
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

```

parser层:
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
