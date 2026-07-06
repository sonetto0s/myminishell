# architecture文件说明

## 本文件用以记录此项目整体架构实现,整体运行逻辑

## V0.2.1 架构
```
主程序:
main
 |
shell_init  (初始化)
 |
shell_run (调用shell层)
 |
shell_cleanup  (清理数据并结束)



shell层(shell_run):
Command com     (定义com结构体用以存放指令)
 |
fgets           (读取键盘输入指令)
 |
trim_line       (去除指令末尾的'\n',转化为'\0')
 |
parse_line      (实现字符串的分离解析,转化为字符后存入com结构体)
|
execute         (实现进程开启,真正开启shell)
 |
is_exit         (测试程序,此处函数只为当前版本需求暂时使用,后续会优化)
```

parser层:
tokenize        (引入tokenize,优化输入字符解析)
 |         
build_command   (将结构体填充,传导至parse_line函数)
 |
parse_line      (实现tokenize和build_command函数调用)