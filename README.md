# memAccessCmd
memAccessCmd是用于SylixOS下对内存/寄存器进行访问和测试的工具。

可以将mem.c编译到app,内核模块或bsp中，方便不同应用情景下调用。本Git仓库是按app工程编译、部署、测试的。

本模块也会不定时更新优化，最新版本可从Git仓库拉取。
Git仓库地址：https://github.com/scilogyhunter/mem.git

# 用法

 -   -h 用于显示命令用法
 -   -i 用于显示命令参数值 其他选项则都是对内存进行操作，默认是按字节（8位）进行，也可以按指定位数来操作，当前支持8位，16位和32位。
 -   -r 用于读取并显示内存内容。
 -   -w 用于向目标地址写入目标内容。
 -   -s 用于将一段内存设置为目标值，类似`PVOID  lib_memset (PVOID  pvDest, INT  iC, size_t  stCount)`函数。
 -   -g 从一个固定地址获取一段内存值，比如从一个FIFO寄存器读取一段数据。
 -   -p 向一个固定地址写入一段内存值，比如向一个FIFO寄存器写入一段数据。
 -   -c 把一段内存值拷贝到另一段内存。
 -   -m 比较两段内存并打印出不相同的部分。
 -  -t 对一段内存进行读写访问测试，如果目标内存/寄存器为只读或只写模式，测试就会报错；如果是非法空间或没有进行内存映射的空间则可能触发系统异常。
 -  -e 对一段内存进行读写速度测试，输出读写速度。


具体操作实例如下：

```bash
[root@sylixos:/root]# /apps/mem/mem -h
Usage:  
mem  -h display help info.
mem  -i display arg  info.
mem  -r[8/16/32] <saddr> <length>         memory read
mem  -w[8/16/32] <daddr> <d0> [d1] [d2]...memory write
mem  -s[8/16/32] <daddr> <value> <length> memory set
mem  -g[8/16/32] <daddr> <saddr> <length> memory get
mem  -p[8/16/32] <daddr> <saddr> <length> memory put
mem  -c[8/16/32] <daddr> <saddr> <length> memory copy
mem  -m[8/16/32] <daddr> <saddr> <length> memory compare
mem  -t[8/16/32] <daddr> <length>         memory access test
mem  -e[8/16/32] <daddr> <length>         memory speed test
[root@sylixos:/root]# 
```

```c
[root@sylixos:/root]# /apps/mem/mem -i
cOperate = r
uiFormat = 1
uiSaddr  = 00000000
uiDaddr  = 00000000
uiValue  = 00000000
uiLength = 64
[root@sylixos:/root]# 
```

```bash
[root@sylixos:/root]# /apps/mem/mem -r 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 1

[41765030] 2f 75 73 72 2f 62 69 6e 00 2f 62 69 6e 00 2f 75    /usr/bin`/bin`/u
[41765040] 73 72 2f 70 6b 67 2f 73 62 69 6e 00 2f 73 62 69    sr/pkg/sbin`/sbi
[root@sylixos:/root]# 
```

```bash
[root@sylixos:/root]# /apps/mem/mem -r8 0x41765030 32  
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 1

[41765030] 2f 75 73 72 2f 62 69 6e 00 2f 62 69 6e 00 2f 75    /usr/bin`/bin`/u
[41765040] 73 72 2f 70 6b 67 2f 73 62 69 6e 00 2f 73 62 69    sr/pkg/sbin`/sbi
[root@sylixos:/root]# 
[root@sylixos:/root]# /apps/mem/mem -r16 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 2

[41765030] 2f75 7372 2f62 696e 002f 6269 6e00 2f75    /usr/bin`/bin`/u
[41765040] 7372 2f70 6b67 2f73 6269 6e00 2f73 6269    sr/pkg/sbin`/sbi
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 2f757372 2f62696e 002f6269 6e002f75    /usr/bin`/bin`/u
[41765040] 73722f70 6b672f73 62696e00 2f736269    sr/pkg/sbin`/sbi
[root@sylixos:/root]# 
```

```bash
[root@sylixos:/root]# /apps/mem/mem -w 0x41765030 1 2 3 4
memory write.
uiDaddr = 41765030, uiLength = 00000004, uiFormat = 1

[41765030] 01 02 03 04                                        ````
[root@sylixos:/root]# /apps/mem/mem -r 0x41765030 32 
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 1

[41765030] 01 02 03 04 2f 62 69 6e 00 2f 62 69 6e 00 2f 75    ````/bin`/bin`/u
[41765040] 73 72 2f 70 6b 67 2f 73 62 69 6e 00 2f 73 62 69    sr/pkg/sbin`/sbi
[root@sylixos:/root]# 
[root@sylixos:/root]# /apps/mem/mem -w16 0x41765030 1 2 3 4
memory write.
uiDaddr = 41765030, uiLength = 00000004, uiFormat = 2

[41765030] 0001 0002 0003 0004                        `````````/bin`/u
[root@sylixos:/root]# /apps/mem/mem -r16 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 2

[41765030] 0001 0002 0003 0004 002f 6269 6e00 2f75    `````````/bin`/u
[41765040] 7372 2f70 6b67 2f73 6269 6e00 2f73 6269    sr/pkg/sbin`/sbi
[root@sylixos:/root]# 
[root@sylixos:/root]# /apps/mem/mem -w32 0x41765030 1 2 3 4
memory write.
uiDaddr = 41765030, uiLength = 00000004, uiFormat = 4

[41765030] 00000001 00000002 00000003 00000004    ````````````````
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 00000001 00000002 00000003 00000004    ````````````````
[41765040] 73722f70 6b672f73 62696e00 2f736269    sr/pkg/sbin`/sbi
[root@sylixos:/root]# 
```

```bash
[root@sylixos:/root]# 
[root@sylixos:/root]# /apps/mem/mem -s 0x41765030 0x55 16
memory set.
uiAddr = 41765030, uiValue = 00000055, uiLength = 00000010, uiFormat = 1
[root@sylixos:/root]# /apps/mem/mem -r 0x41765030 32  
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 1

[41765030] 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55 55    UUUUUUUUUUUUUUUU
[41765040] 73 72 2f 70 6b 67 2f 73 62 69 6e 00 2f 73 62 69    sr/pkg/sbin`/sbi
[root@sylixos:/root]# /apps/mem/mem -s16 0x41765030 0x66 16
memory set.
uiAddr = 41765030, uiValue = 00000066, uiLength = 00000010, uiFormat = 2
[root@sylixos:/root]# /apps/mem/mem -r16 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 2

[41765030] 0066 0066 0066 0066 0066 0066 0066 0066    `f`f`f`f`f`f`f`f
[41765040] 7372 2f70 6b67 2f73 6269 6e00 2f73 6269    sr/pkg/sbin`/sbi
[root@sylixos:/root]# /apps/mem/mem -s32 0x41765030 0x12345678 16
memory set.
uiAddr = 41765030, uiValue = 12345678, uiLength = 00000010, uiFormat = 4
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[41765040] 73722f70 6b672f73 62696e00 2f736269    sr/pkg/sbin`/sbi
[root@sylixos:/root]# ```

```bash
[root@sylixos:/root]# /apps/mem/mem -g 0x41765040 0x41765030 16
memory get.
uiDaddr = 41765040, uiSaddr = 41765030, uiLength = 00000010, uiFormat = 1
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[41765040] 12121212 12121212 12121212 12121212    ````````````````
[root@sylixos:/root]# /apps/mem/mem -g16 0x41765040 0x41765030 16
memory get.
uiDaddr = 41765040, uiSaddr = 41765030, uiLength = 00000010, uiFormat = 2
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[41765040] 12341234 12341234 12341234 12341234    `4`4`4`4`4`4`4`4
[root@sylixos:/root]# /apps/mem/mem -g32 0x41765040 0x41765030 16
memory get.
uiDaddr = 41765040, uiSaddr = 41765030, uiLength = 00000010, uiFormat = 4
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[41765040] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[root@sylixos:/root]# 
```

```bash
[root@sylixos:/root]# /apps/mem/mem -p 0x41765040 0x41765030 16
memory put.
uiDaddr = 41765040, uiSaddr = 41765030, uiLength = 00000010, uiFormat = 1
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 78345678 12345678 12345678 12345678    x4Vx`4Vx`4Vx`4Vx
[41765040] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[root@sylixos:/root]# /apps/mem/mem -p16 0x41765040 0x41765030 16
memory put.
uiDaddr = 41765040, uiSaddr = 41765030, uiLength = 00000010, uiFormat = 2
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 56785678 12345678 12345678 12345678    VxVx`4Vx`4Vx`4Vx
[41765040] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[root@sylixos:/root]# /apps/mem/mem -p32 0x41765040 0x41765030 16
memory put.
uiDaddr = 41765040, uiSaddr = 41765030, uiLength = 00000010, uiFormat = 4
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[41765040] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[root@sylixos:/root]# 
```

```bash
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 5a5a5a5a 5a5a5a5a 5a5a5a5a 5a5a5a5a    ZZZZZZZZZZZZZZZZ
[41765040] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[root@sylixos:/root]# 
[root@sylixos:/root]# /apps/mem/mem -c 0x41765030 0x41765040 16  
memory copy.
uiDaddr = 41765030, uiSaddr = 41765040, uiLength = 00000010, uiFormat = 1
[root@sylixos:/root]# /apps/mem/mem -r32 0x41765030 32
memory read.
uiSaddr = 41765030, uiLength = 00000020, uiFormat = 4

[41765030] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[41765040] 12345678 12345678 12345678 12345678    `4Vx`4Vx`4Vx`4Vx
[root@sylixos:/root]# 
```

```bash
[root@sylixos:/root]# /apps/mem/mem -m 0x41765030 0x41765040 16
memory compare
uiDaddr = 41765030, uiSaddr = 41765040, uiLength = 00000010, uiFormat = 1
different count is 0
[root@sylixos:/root]# /apps/mem/mem -w32 0x41765030 0x5a5a5a5a   
memory write.
uiDaddr = 41765030, uiLength = 00000001, uiFormat = 4

[41765030] 5a5a5a5a                               ZZZZ`4Vx`4Vx`4Vx
[root@sylixos:/root]# /apps/mem/mem -m 0x41765030 0x41765040 16
memory compare
uiDaddr = 41765030, uiSaddr = 41765040, uiLength = 00000010, uiFormat = 1
[41765030 -- 41765040] 5a -- 12
[41765031 -- 41765041] 5a -- 34
[41765032 -- 41765042] 5a -- 56
[41765033 -- 41765043] 5a -- 78
different count is 4
[root@sylixos:/root]# /apps/mem/mem -m16 0x41765030 0x41765040 16
memory compare
uiDaddr = 41765030, uiSaddr = 41765040, uiLength = 00000010, uiFormat = 2
[41765030 -- 41765040] 5a5a -- 1234
[41765032 -- 41765042] 5a5a -- 5678
different count is 2
[root@sylixos:/root]# 
[root@sylixos:/root]# /apps/mem/mem -m32 0x41765030 0x41765040 16
memory compare
uiDaddr = 41765030, uiSaddr = 41765040, uiLength = 00000010, uiFormat = 4
[41765030 -- 41765040] 5a5a5a5a -- 12345678
different count is 1
[root@sylixos:/root]# 
```

```bash
[root@sylixos:/root]# /apps/mem/mem -t 0x41765030 1024           
memory access test
uiDaddr = 41765030, uiLength = 00000400, uiFormat = 1
[root@sylixos:/root]# 
[root@sylixos:/root]# /apps/mem/mem -e 0x41765030 1024
memory speed test
uiDaddr = 41765030, uiLength = 00000400, uiFormat = 1
memspeed read        1024 byte by         47 us (  21.79MBps)
memspeed write       1024 byte by         29 us (  35.31MBps)
[root@sylixos:/root]# 
```
