#  debug_log文件说明

## 本文件用以记录项目实现中遇到的调试问题等(后半部分已经当日记用了 :))

## 调试问题

### 现象:输入指令均返回错误address
### 缘由与解决过程:
```
parser.c文件中,parse函数在最后时候忘了写结构体末尾指向NULL,导致结构体没有收尾,传导入shell文件中后,引起execvp函数参数传递错误。后在parser.c文件里面修改原有结构体逻辑漏洞后问题解决

下面是GDB调试的部分代码,能看出来execvp参数明显不正常☠️

(gdb) print com
$1 = (Command *) 0x7fffffffdb20
(gdb) print *com
$2 = {argc = 1, argv = {0x7fffffffdb80 "ls", 0x555555556004 ">>shell初始化成功\r", 
    0x7ffff7e045c0 <_IO_2_1_stdout_> "\204*\255", <incomplete sequence \373>, 0xa <error: Cannot access memory at address 0xa>, 
    0x7ffff7e045c0 <_IO_2_1_stdout_> "\204*\255", <incomplete sequence \373>, 0x7ffff7e046a8 <stdout> "\300E\340\367\377\177", 
    0x7ffff7e02030 <_IO_file_jumps> "", 0x7fffffffdb90 "\340\333\377\377\377\177", 
    0x7ffff7c92ef3 <_IO_new_file_overflow+259> "\203\370\377\017\205c\377\377\377\017\037@", 
    0x555555556004 ">>shell初始化成功\r"}}
(gdb) print com->argv
$3 = {0x7fffffffdb80 "ls", 0x555555556004 ">>shell初始化成功\r", 
  0x7ffff7e045c0 <_IO_2_1_stdout_> "\204*\255", <incomplete sequence \373>, 0xa <error: Cannot access memory at address 0xa>, 
  0x7ffff7e045c0 <_IO_2_1_stdout_> "\204*\255", <incomplete sequence \373>, 0x7ffff7e046a8 <stdout> "\300E\340\367\377\177", 
  0x7ffff7e02030 <_IO_file_jumps> "", 0x7fffffffdb90 "\340\333\377\377\377\177", 
  0x7ffff7c92ef3 <_IO_new_file_overflow+259> "\203\370\377\017\205c\377\377\377\017\037@", 
  0x555555556004 ">>shell初始化成功\r"}

```
## 随笔(或者说可以算日记)

```
- 这dispatcher可太棒了,几乎完美解决了ifelse写一大段的痛点(而且看着好有逼格),这种高效有用的工程模板真是好极了🫠

- 暑假的时间就是多啊~ 一天时间再加上之前积累,推进喜人啊😎更关键的是全是我自己敲的,感觉学到好多啊

- 这GDB调试结果看着吓人,结果细看一下,合着一大坨本来就是无意义乱码,问题暴露的挺明显,问题还挺好解决的(虽然这问题本来就不该出现,忘写结构体的收尾纯纯是我脑子抽了。。。)

- OK啊,我的第一个正式的linux应用层的项目顺利开张啦(please无视那个原来的minishell),不知道啥时候能做完,因为做到V1.0后,我想把他往设备类型那种linux应用转一点,可能要做到8月多？不过9月前肯定能结束😎(要是结束不了那我后面的规划项目就全乱了)
```