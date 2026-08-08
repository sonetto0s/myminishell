# test文件说明

## 本文件用以记录测试指令运行结果,验证当前版本效果

## 当前版本 : V1.1

## 运行环境
- 系统:Ubuntu 22.04
- 编译方式:gcc(Makefile)
- 运行软件: Visual Studio Code (ssh)
- c11

## 测试指令输出

```
ls :
build  common  docs  include  Makefile  README.md  shell  src


ls -l:
drwxrwxr-x 4 matilda matilda  4096 Jul 28 13:26 build
drwxrwxr-x 2 matilda matilda  4096 Jul  7 14:08 common
drwxrwxr-x 2 matilda matilda  4096 Jul 28 13:21 docs
drwxrwxr-x 2 matilda matilda  4096 Jul 26 20:12 include
-rw-rw-r-- 1 matilda matilda   534 Jul 26 20:12 Makefile
-rw-rw-r-- 1 matilda matilda  3346 Jul 28 13:21 README.md
-rwxrwxr-x 1 matilda matilda 40584 Jul 28 13:26 shell
drwxrwxr-x 2 matilda matilda  4096 Jul 26 20:12 src


echo hello > test.txt:
hello成功写入test文件


echo world >> test.txt:
world顺利写入跟在hello后


cat < test.txt:
hello
world


ls | grep txt:
test.txt


cat test.txt | grep hello | wc -l:
1


ls 
echo $?:
0


Ctrl + C测试:
sleep 100:
^C
>>MiniShell


Ctrl + D测试:
>>MiniShell 已退出


```

## 全部指令通过检验,shell可正常执行操作
 