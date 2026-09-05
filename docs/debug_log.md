#  debug_log文件说明

## 本文件用以记录项目实现中遇到的调试问题等(后半部分已经当日记用了 :))

## 调试问题

###　现象:cat指令无效
###  缘由与解决过程:
```
在command.c文件中对指令结构体进行重新初始化后,忘记了在shell.c函数对创建的com结构体调用这个初始化函数,使得输入结构体内容实质上为垃圾值,原有错误程序执行后不仅cat指令没有任何输出,还创建了各种乱码名字的文件

现在已经修补了逻辑漏洞,经测试,代码运行正常,cat指令正常

```

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

## 现象:github仓库破损(或者说.git损坏)
### 缘由与解决过程:

```
也不知道这个能不能算是调试问题，不过反正都写在这里吧
这问题我还真是头一次见,压根没想过git能给我拉一坨大的，我现在实际上还不能100%确定问题在哪,但是我基本锁定了范围
80%可能是因为这个破Ubuntu关机或者死机了,然后我是ssh连接的他在windows里面写的,结果一顿操作下,.git/objects里的东西直接变成棍母了,啥也没了，剩下20%可能估计就是这文件自己抽风了
解决过程其实也很简单,这个不算是什么特别严重问题,当然我说的是从解决难度上,单纯看仓库损坏还是挺吓人的.解决思路也挺好想到,只是.git损坏，但是源码什么的都在,原有仓库也还能打开.打个比方就是,一本书，内容都还在，但是目录被撕了(诶好像也有点不咋像,不过反正就那意思)，反正就是现在我的代码和仓库间原有的git联系断了,那解决办法就是再重新做一个联系,原有git直接rm -rf，重新init一下，然后重新绑定仓库，then就OK了,静下心来，解决还是好解决的

哦另外还学到一个别的小知识，好像也不算，嗯我也不知道为啥我现在想写出来,
这个github认证现在咋还不用密码了🤔,也开始用上token/key了,这么紧跟时代潮流吗

```

## 现象:运行下列指令后程序意外退出:sleep 20    ctrl+Z    jobs
### 缘由与解决过程:

```
这个算是一个设计问题,就按我正常构思,这个终端运行应该是这样:我输入sleep 20,然后按下Ctrl+z,那等于是暂停嘛,然后输入jobs,正常来说那就是返回状态,现在这个进程怎么样怎么样,但是我的两个文件中的代码设计有个bug,我也不写啥专门术语啥的烂七八糟的了,那调试总结是给人越看越明白的不是越看越乱的
let me organize一下语言,简单说就是我这个sleep 20输入后,然后按下Ctrl+Z,这个时候,execute文件里面有个专门管这个的函数,也就是那个signle里面的waitpid()那个函数,具体咋写我懒得翻了,反正你肯定能找到,这个函数就是负责等进程结束或者接收被暂停的信号,诶接收信号这个词好像有点不严谨,但反正意思是这个样,你知道我在说啥就行,然后有意思的来了,真信号sigchld在ctrl+z后会把信息传给shell,然后shell经过各种调用会用到一个job_reap函数,正好他里面也有一个waitpid,这下他也去"接收信号",那他肯定是啥也接不到的,消息已经被上一个waitpid拿走了,然后这玩意就返回-1,然后被调用判断返回值后,程序惊奇的发现完美符合errno🤓,so他直接给这个进程判定为似了,然后jobs一运行发现是对一个被认为已经死了的东西进行操作,然后,ha

解决方法很简单,加个判断就行,反正只要让系统不做成那种发现没信息就统一认为肯定是似了就行

哦操作时候的运行指令代码如下
matilda@LAPTOP-UJB0D4DS:~/workspace/myminishell$ ./shell
[INFO] >>shell初始化成功
>>MiniShell pwd
/home/matilda/workspace/myminishell
>>MiniShell sleep 20
^Z
[1]+ Stopped sleep
>>MiniShell jobs
>>MiniShell 已退出
matilda@LAPTOP-UJB0D4DS:~/workspace/myminishell$ jobs
matilda@LAPTOP-UJB0D4DS:~/workspace/myminishell$ grep -R "builtin_jobs" -n src include
src/builtin_table.c:10:    {"jobs", builtin_jobs},
src/builtin.c:73:int builtin_jobs(Command *cmd, struct ShellContext *ctx)
include/builtin.h:12:int builtin_jobs(Command *cmd, struct ShellContext *ctx);
matilda@LAPTOP-UJB0D4DS:~/workspace/myminishell$ grep -R "job_list" -n src include
src/job.c:51:void job_list(JobManager *manager)
src/builtin.c:76:    job_list(&ctx->jobs);
include/job.h:45:void job_list(JobManager *manager);

matilda@LAPTOP-UJB0D4DS:~/workspace/myminishell$ make clean
rm -rf build
rm -f shell
rm -f test
matilda@LAPTOP-UJB0D4DS:~/workspace/myminishell$ make debug
make clean
make[1]: Entering directory '/home/matilda/workspace/myminishell'
rm -rf build
rm -f shell
rm -f test
make[1]: Leaving directory '/home/matilda/workspace/myminishell'
make \
CFLAGS="-Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address" \
LDFLAGS="-fsanitize=address"
make[1]: Entering directory '/home/matilda/workspace/myminishell'
CC src/main.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/main.c -o build/src/main.o
CC src/shell.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/shell.c -o build/src/shell.o
CC src/parser.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/parser.c -o build/src/parser.o
CC src/executor.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/executor.c -o build/src/executor.o
CC src/dispatcher.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/dispatcher.c -o build/src/dispatcher.o
CC src/builtin.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/builtin.c -o build/src/builtin.o
CC src/command.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/command.c -o build/src/command.o
CC src/sig.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/sig.c -o build/src/sig.o
CC src/shell_context.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/shell_context.c -o build/src/shell_context.o
CC src/job.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/job.c -o build/src/job.o
CC src/event.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/event.c -o build/src/event.o
CC src/builtin_table.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/builtin_table.c -o build/src/builtin_table.o
CC common/utils.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c common/utils.c -o build/common/utils.o
CC common/log.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c common/log.c -o build/common/log.o
CC common/error.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c common/error.c -o build/common/error.o
CC config/config.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c config/config.c -o build/config/config.o
CC src/system_info.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/system_info.c -o build/src/system_info.o
CC src/terminal.c
gcc -Wall -Wextra -g -Iinclude -Icommon -Iconfig -MMD -MP -fsanitize=address -c src/terminal.c -o build/src/terminal.o
LD shell
gcc build/src/main.o build/src/shell.o build/src/parser.o build/src/executor.o build/src/dispatcher.o build/src/builtin.o build/src/command.o build/src/sig.o build/src/shell_context.o build/src/job.o build/src/event.o build/src/builtin_table.o build/common/utils.o build/common/log.o build/common/error.o build/config/config.o build/src/system_info.o build/src/terminal.o -fsanitize=address -o shell
make[1]: Leaving directory '/home/matilda/workspace/myminishell'
matilda@LAPTOP-UJB0D4DS:~/workspace/myminishell$ ./shell
[INFO] >>shell初始化成功
>>MiniShell pwd
/home/matilda/workspace/myminishell
>>MiniShell sleep 20
^Z
[1]+ Stopped sleep
>>MiniShell jobs
[1] sleep
now it is stopped
>>MiniShell ^C
>>MiniShell 已退出
matilda@LAPTOP-UJB0D4DS:~/workspace/myminishell$


```

## 随笔(或者说可以算日记)

```
- 实在懒得把architecture翻新一遍了,直接ai得了算了

-好吧还是写吧,说啥来啥,这新bug折腾了半天🤡

- 这调试越来越不想写了,就我一个人调试啥。。。何况现在9成出错都是少打个字母这种的,真写不了一点调试

- 要不以后还是写英文算了,英文比中文看着正式一点

- 我咋又忘了更新README呢

- 哎不是,我咋清楚地记得我写过一条评论了呢,去哪了

- 这个commit突然转变一下上传思维还是有点不习惯啊

- 好像很久没更新过这个调试遇到的问题了,不过现在阶段,绝大部分问题都是小毛病,稍微改下就行,或者喂给ai直接完全解决,好像很久都遇不到那种能让我卡死半天的调试bug了

- 现在似乎,随便给我一个代码,我好像都能把他拆解下来然后慢慢看明白,或者说我起码知道他在干啥了,不像当初给个代码那真是大眼瞪小眼,似乎有种半只脚踏入工程师境界的感觉？

- 这openai真无敌了,一断订阅,ai模型立马降智

- 依旧爆改makefile中....

- 退出码好怪哦,return 128 + WTERMSIG(last_status);这种加上一个码数的代码倒是有点意思🤔

- 搓了四个小时,总算基本做好了

- 这个破架构好难写啊。。。。。不是咋就这么多东西捏

- 好多啊...不过我认了,这个算是个收官🙃必须得弄好点

- 这个测试先不写那么多了,后续慢慢维护吧。我快燃尽了

- 这V1.0东西好多啊,或者说把一个大版本的东西塞进一个v里面真的好多东西啊。

- oh,又学了个好东西

- github快给我连上啊啊啊啊。。又push不了了

- 要不我研究研究咋把图片弄上来,要是能弄上来那就有意思了

- 这github的project,做完一个模块,把他拖到done上面有种打怪闯关的成就感(‾◡◝)

- 这github界面挺好玩🫠

- 让我想想该咋一边高效使用ai一边能不被他影响的变成只会借鉴,不过好像我现在这个阶段也只能先借鉴？某种意义上这跟找开源然后自己学一毛一样

- 感觉很多大毛病现在不咋见了,小毛病倒是基本没啥好记录的(*￣0￣),诶这颜文字挺好玩

- 又学了一些新函数,ha😎

- 一日好过一日,一年胜似一年~

- 这个死虚拟机我忍你很久了。。。。。。

- 想了想，这种跟着ai一步一步做的项目，感觉到后期自己见的东西多了，似乎也应该试试纯自己敲了？next试试吧🙃

- 简单的项目更新起来还是挺快的啊,果然杂事少了后专心做一样东西就是效率高

- 我想想,也许到8月初就可以做到1.0版本了,后续看看怎么维护升级吧

- 画板子这种轮椅玩多了,突然看上代码脑子有种转圈转不起来的感觉.....

- 不好意思啊,从7月7号开始我就不在学校了,就去南宁了,当时是要做一个项目拖了一段时间,这个项目就中途断了一下,结果做完后正好深圳有家公司找到我们实验室的人培训,本来是两个研究生学长要去,最后老师让我也跟着去学习了(画一个很有意思的板子😎),这个项目就接着延期了,一直到今天板子画完了等打板,总算有时间接着做了

- 这结构体可真是个尤物啊哈,太好用辣

- 这个architecture文件居然一直忘了改版本号。。。算了，下次记住

- 我发现只要出现的问题特别离谱,而且一次小更新能变出来一大堆问题那种,多半就是函数没初始化弄了一堆垃圾值☠️

- 导头文件导力竭了。。。真就牵一发而动全身

- 这dispatcher可太棒了,几乎完美解决了ifelse写一大段的痛点(而且看着好有逼格),这种高效有用的工程模板真是好极了🫠

- 暑假的时间就是多啊~ 一天时间再加上之前积累,推进喜人啊😎更关键的是全是我自己敲的,感觉学到好多啊

- 这GDB调试结果看着吓人,结果细看一下,合着一大坨本来就是无意义乱码,问题暴露的挺明显,问题还挺好解决的(虽然这问题本来就不该出现,忘写结构体的收尾纯纯是我脑子抽了。。。)

- OK啊,我的第一个正式的linux应用层的项目顺利开张啦(please无视那个原来的minishell),不知道啥时候能做完,因为做到V1.0后,我想把他往设备类型那种linux应用转一点,可能要做到8月多？不过9月前肯定能结束😎(要是结束不了那我后面的规划项目就全乱了)
```
