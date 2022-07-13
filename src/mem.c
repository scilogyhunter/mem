/*********************************************************************************************************
**
**                                    中国软件开源组织
**
**                                   嵌入式实时操作系统
**
**                                       SylixOS(TM)
**
**                               Copyright  All Rights Reserved
**
**--------------文件信息--------------------------------------------------------------------------------
**
** 文   件   名: mem.c
**
** 创   建   人: Hou.JinYu (侯进宇)
**
** 文件创建日期: 2017 年 10 月 17 日
**
** 描        述: 通用系统内存访问命令，可以直接读取或修改内存值，用于调试
*********************************************************************************************************/
#include "SylixOS.h"                                                    /*  操作系统                    */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
/*********************************************************************************************************
  宏定义
*********************************************************************************************************/
#define   BUF_SIZE              (256)                                   /*  缓冲区大小                  */
/*********************************************************************************************************
  PRINT 打印宏函数
*********************************************************************************************************/
#define PRINT_INFO(fmt, ...)    do {printf(fmt, ##__VA_ARGS__);} while (0)
/*********************************************************************************************************
  读写宏函数
*********************************************************************************************************/
#if !defined(read8) && !defined(write8)
#define read8(a)                (*(volatile UINT8  *)(a))
#define read16(a)               (*(volatile UINT16 *)(a))
#define read32(a)               (*(volatile UINT32 *)(a))
#define read64(a)               (*(volatile UINT64 *)(a))

#define write8(d, a)            (*(volatile UINT8  *)(a) = (d))
#define write16(d, a)           (*(volatile UINT16 *)(a) = (d))
#define write32(d, a)           (*(volatile UINT32 *)(a) = (d))
#define write64(d, a)           (*(volatile UINT64 *)(a) = (d))
#endif
/*********************************************************************************************************
  操作宏
*********************************************************************************************************/
#define MEMTEST(xx)                                                                               \
    t[0] = getTimeUs();                                                                           \
    for (i = 0; i < uiCount; i++) {                                                               \
        read##xx(ulAddr);read##xx(ulAddr);read##xx(ulAddr);read##xx(ulAddr);                      \
        read##xx(ulAddr);read##xx(ulAddr);read##xx(ulAddr);read##xx(ulAddr);                      \
        read##xx(ulAddr);read##xx(ulAddr);read##xx(ulAddr);read##xx(ulAddr);                      \
        read##xx(ulAddr);read##xx(ulAddr);read##xx(ulAddr);read##xx(ulAddr);                      \
    }                                                                                             \
    t[1] = getTimeUs();                                                                           \
    time[0] = t[1] - t[0];                                                                        \
    v = read##xx(ulAddr);                                                                         \
    t[0] = getTimeUs();                                                                           \
    for (i = 0; i < uiCount; i++) {                                                               \
        write##xx(v, ulAddr);write##xx(v, ulAddr);write##xx(v, ulAddr);write##xx(v, ulAddr);      \
        write##xx(v, ulAddr);write##xx(v, ulAddr);write##xx(v, ulAddr);write##xx(v, ulAddr);      \
        write##xx(v, ulAddr);write##xx(v, ulAddr);write##xx(v, ulAddr);write##xx(v, ulAddr);      \
        write##xx(v, ulAddr);write##xx(v, ulAddr);write##xx(v, ulAddr);write##xx(v, ulAddr);      \
    }                                                                                             \
    t[1] = getTimeUs();                                                                           \
    time[1] = t[1] - t[0];
/*********************************************************************************************************
  参数结构体
*********************************************************************************************************/
typedef struct {
    CHAR         cOperate;                                              /*  操作类型r w s c t           */
    UINT32       uiFormat;                                              /*  1:u8 2:u16 4:u32            */
    UINT32       uiSaddr;                                               /*  源地址                      */
    UINT32       uiDaddr;                                               /*  目的地址                    */
    UINT32       uiValue;                                               /*  数值                        */
    UINT32       uiLength;                                              /*  数据长度                    */
    UINT32       uiBuf[BUF_SIZE];                                       /*  缓冲区                      */
} ARG_ST;
/*********************************************************************************************************
** 函数名称: getTimeUs
** 功能描述: 获取系统运行时间，单位微秒
** 输    入: 无
** 输    出: 系统运行时间
*********************************************************************************************************/
static UINT64  getTimeUs (VOID)
{
    struct timespec  tv;

    lib_clock_gettime(CLOCK_MONOTONIC, &tv);

    return  (tv.tv_sec * 1000000 + tv.tv_nsec / 1000);
}
/*********************************************************************************************************
** 函数名称: memoryHelp
** 功能描述: 帮助信息
** 输　  入: NONE
** 输    出: NONE
*********************************************************************************************************/
static VOID  memoryHelp (VOID)
{
    PRINT_INFO("Usage:  \n"
       "mem  -h display help info.\n"
       "mem  -i display arg  info.\n"
       "mem  -r[8/16/32] <saddr> <length>         memory read\n"
       "mem  -w[8/16/32] <daddr> <d0> [d1] [d2]...memory write\n"
       "mem  -s[8/16/32] <daddr> <value> <length> memory set\n"
       "mem  -g[8/16/32] <daddr> <saddr> <length> memory get\n"
       "mem  -p[8/16/32] <daddr> <saddr> <length> memory put\n"
       "mem  -c[8/16/32] <daddr> <saddr> <length> memory copy\n"
       "mem  -m[8/16/32] <daddr> <saddr> <length> memory compare\n"
       "mem  -t[8/16/32] <daddr> <length>         memory access test\n"
       "mem  -e[8/16/32] <daddr> <length>         memory speed test\n");
}
/*********************************************************************************************************
** 函数名称: memoryInfo
** 功能描述: 参数信息
** 输　  入: pArg  参数
** 输    出: NONE
*********************************************************************************************************/
static VOID  memoryInfo (ARG_ST  *pArg)
{
    UINT32   i;

    PRINT_INFO("cOperate = %c\n",   pArg->cOperate);
    PRINT_INFO("uiFormat = %d\n",   pArg->uiFormat);
    PRINT_INFO("uiSaddr  = %08x\n", pArg->uiSaddr);
    PRINT_INFO("uiDaddr  = %08x\n", pArg->uiDaddr);
    PRINT_INFO("uiValue  = %08x\n", pArg->uiValue);
    PRINT_INFO("uiLength = %d\n",   pArg->uiLength);

    if (pArg->uiLength && (pArg->cOperate == 'w')) {
        for (i = 0; i < pArg->uiLength; i++) {
            if ((i % 16) == 0) {
                PRINT_INFO("\n[%08x] ", i);
            }

            PRINT_INFO(" %02x", pArg->uiBuf[i]);
        }

        PRINT_INFO("\n");
    }
}
/*********************************************************************************************************
** 函数名称: memoryParseArg
** 功能描述: 参数解析
** 输　入  : pArg  参数
**           iArgC
**           ppcArgV
** 输　出  : ERROR_CODE
*********************************************************************************************************/
static INT  memoryParseArg (ARG_ST  *pArg, INT  iArgC, PCHAR  ppcArgV[])
{
    UINT32  i;

    if (iArgC < 2) {
        return  (PX_ERROR);
    }

    if (ppcArgV[1][0] != '-') {
        return  (PX_ERROR);
    }

    pArg->cOperate = ppcArgV[1][1];

    if (strlen(ppcArgV[1]) > 2) {
        pArg->uiFormat = strtoul(&ppcArgV[1][2], NULL, 10) / 8;
    } else {
        pArg->uiFormat = 1;
    }


    if (pArg->cOperate == 'r') {
        if (iArgC < 4) {
            return  (PX_ERROR);
        }

        pArg->uiSaddr  = strtoul(ppcArgV[2], NULL, 0);
        pArg->uiLength = strtoul(ppcArgV[3], NULL, 0);


    } else if (pArg->cOperate == 'w') {
        if (iArgC < 4) {

            return  (PX_ERROR);
        }

        pArg->uiDaddr = strtoul(ppcArgV[2], NULL, 0);
        pArg->uiLength = ((iArgC - 3) < BUF_SIZE) ? (iArgC - 3) : BUF_SIZE;

        for (i = 0; i < pArg->uiLength; i++) {
            pArg->uiBuf[i] = strtoul(ppcArgV[3 + i], NULL, 0);
        }

    } else if (pArg->cOperate == 's') {
        if (iArgC < 5) {

            return  (PX_ERROR);
        }

        pArg->uiDaddr  = strtoul(ppcArgV[2], NULL, 0);
        pArg->uiValue  = strtoul(ppcArgV[3], NULL, 0);
        pArg->uiLength = strtoul(ppcArgV[4], NULL, 0);

    } else if ((pArg->cOperate == 'g') || (pArg->cOperate == 'p') || (pArg->cOperate == 'c') ||
               (pArg->cOperate == 'm')) {
        if (iArgC < 5) {

            return  (PX_ERROR);
        }

        pArg->uiDaddr  = strtoul(ppcArgV[2], NULL, 0);
        pArg->uiSaddr  = strtoul(ppcArgV[3], NULL, 0);
        pArg->uiLength = strtoul(ppcArgV[4], NULL, 0);
    } else if ((pArg->cOperate == 't') || (pArg->cOperate == 'e')) {
        if (iArgC < 4) {
            return  (PX_ERROR);
        }

        pArg->uiDaddr  = strtoul(ppcArgV[2], NULL, 0);
        pArg->uiLength = strtoul(ppcArgV[3], NULL, 0);
    } else {
        return  (PX_ERROR);
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memRead
** 功能描述: 读取并打印内存
** 输    入: uiSaddr        源地址
**           uiLength       长度
**           uiFormat       格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static INT  memRead (UINT32  uiSaddr, UINT32  uiLength, UINT32  uiFormat)
{
    CHAR     c;
    UINT32   i;
    UINT32   j;
    UINT32   n;
    UINT32   l;
    UINT32   uiCount;

    volatile  UINT8  *pData08;
    volatile  UINT16 *pData16;
    volatile  UINT32 *pData32;

    if (uiFormat == 1) {
        uiCount = uiLength;
        pData08 = (UINT8  *)uiSaddr;
        for (i = 0; i < uiCount; i+= 16) {
            if ((i % 16) == 0) {
                PRINT_INFO("\n[%08x]", i * uiFormat + uiSaddr);
            }
            n = i;

            if ((uiCount - n) >= 16) {
                l = 16;
            } else {
                l = uiCount - n;
            }

            for (j = 0; j < l; j++) {
                PRINT_INFO(" %02x", pData08[n + j]);
            }
            for (; j < 16; j++) {
                PRINT_INFO("   ");
            }

            PRINT_INFO("    ");
            for (j = 0; j < l; j++) {
                c = pData08[n + j];
                if (isprint(c)) {
                    PRINT_INFO("%c", c);
                } else {
                    PRINT_INFO("`");
                }
            }
            for (; j < 16; j++) {
                PRINT_INFO(" ");
            }
        }
    } else if (uiFormat == 2) {
        uiCount = (uiLength + 1) / 2;
        pData08 = (UINT8  *)uiSaddr;
        pData16 = (UINT16 *)uiSaddr;
        for (i = 0; i < uiCount; i+= 8) {
            if ((i % 8) == 0) {
                PRINT_INFO("\n[%08x]", i * uiFormat + uiSaddr);
            }
            n = i;

            if ((uiCount - n) >= 8) {
                l = 8;
            } else {
                l = uiCount - n;
            }

            for (j = 0; j < l; j++) {
                PRINT_INFO(" %04x", pData16[n + j]);
            }
            for (; j < 8; j++) {
                PRINT_INFO("     ");
            }

            PRINT_INFO("    ");
            for (j = 0; j < 16; j++) {
                c = pData08[n * 2 + j];
                if (isprint(c)) {
                    PRINT_INFO("%c", c);
                } else {
                    PRINT_INFO("`");
                }
            }
            for (; j < 16; j++) {
                PRINT_INFO(" ");
            }
        }

    } else if (uiFormat == 4) {
        uiCount = (uiLength + 3) / 4;
        pData08 = (UINT8  *)uiSaddr;
        pData32 = (UINT32 *)uiSaddr;
        for (i = 0; i < uiCount; i+= 4) {
            if ((i % 4) == 0) {
                PRINT_INFO("\n[%08x]", i * uiFormat + uiSaddr);
            }
            n = i;

            if ((uiCount - n) >= 4) {
                l = 4;
            } else {
                l = uiCount - n;
            }

            for (j = 0; j < l; j++) {
                PRINT_INFO(" %08x", pData32[n + j]);
            }
            for (; j < 4; j++) {
                PRINT_INFO("         ");
            }

            PRINT_INFO("    ");
            for (j = 0; j < 16; j++) {
                c = pData08[n * 4 + j];
                if (isprint(c)) {
                    PRINT_INFO("%c", c);
                } else {
                    PRINT_INFO("`");
                }
            }
            for (; j < 16; j++) {
                PRINT_INFO(" ");
            }
        }
    } else {
        memoryHelp();
    }

    PRINT_INFO("\n");


    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memWrite
** 功能描述: 写内存
** 输    入: uiDaddr      目的地址
**           uiLength     长度
**           uiBuf        缓存
**           uiFormat     格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static INT  memWrite (UINT32  uiDaddr, UINT32  uiLength, UINT32  uiBuf[], UINT32  uiFormat)
{
    UINT32   i;
    UINT32   uiCount;

    volatile  UINT8  *pData08;
    volatile  UINT16 *pData16;
    volatile  UINT32 *pData32;

    uiCount = uiLength;
    if (uiFormat == 1) {
        pData08 = (UINT8  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData08[i] = uiBuf[i];
        }

    } else if (uiFormat == 2) {
        pData16 = (UINT16 *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData16[i] = uiBuf[i];
        }
    } else if (uiFormat == 4) {
        pData32 = (UINT32 *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData32[i] = uiBuf[i];
        }
    } else {
        memoryHelp();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memSet
** 功能描述: 设置内存
** 输    入: uiDaddr      目的地址
**           uiLength     长度
**           uiValue      值
**           uiFormat     格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static INT  memSet (UINT32  uiDaddr, UINT32  uiLength, UINT32  uiValue, UINT32  uiFormat)
{
    UINT32   i;
    UINT32   uiCount;

    volatile  UINT8  *pData08;
    volatile  UINT16 *pData16;
    volatile  UINT32 *pData32;

    if (uiFormat == 1) {
        uiCount = uiLength;
        pData08 = (UINT8  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData08[i] = uiValue;
        }

    } else if (uiFormat == 2) {
        uiCount = uiLength / 2;
        pData16 = (UINT16 *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData16[i] = uiValue;
        }
    } else if (uiFormat == 4) {
        uiCount = uiLength / 4;
        pData32 = (UINT32 *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData32[i] = uiValue;
        }
    } else {
        memoryHelp();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memGet
** 功能描述: 设置内存，从一个固定地址读并设置目标内存段
** 输    入: uiDaddr      目的地址
**           uiLength     长度
**           uiSaddr      源地址
**           uiFormat     格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static INT  memGet (UINT32  uiDaddr, UINT32  uiLength, UINT32  uiSaddr, UINT32  uiFormat)
{
    UINT32   i;
    UINT32   uiCount;

    volatile  UINT8  *pData08;
    volatile  UINT16 *pData16;
    volatile  UINT32 *pData32;

    if (uiFormat == 1) {
        uiCount = uiLength;
        pData08 = (UINT8  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData08[i] = *(volatile  UINT8  *)uiSaddr;
        }

    } else if (uiFormat == 2) {
        uiCount = uiLength / 2;
        pData16 = (UINT16 *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData16[i] = *(volatile  UINT16  *)uiSaddr;
        }
    } else if (uiFormat == 4) {
        uiCount = uiLength / 4;
        pData32 = (UINT32 *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pData32[i] = *(volatile  UINT32  *)uiSaddr;
        }
    } else {
        memoryHelp();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memPut
** 功能描述: 设置内存，从一段内存向一个固定地址设置数据
** 输    入: uiDaddr      目的地址
**           uiLength     长度
**           uiSaddr      源地址
**           uiFormat     格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static INT  memPut (UINT32  uiDaddr, UINT32  uiLength, UINT32  uiSaddr, UINT32  uiFormat)
{
    UINT32   i;
    UINT32   uiCount;

    volatile  UINT8  *pData08;
    volatile  UINT16 *pData16;
    volatile  UINT32 *pData32;

    if (uiFormat == 1) {
        uiCount = uiLength;
        pData08 = (UINT8  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            *(volatile  UINT8  *)uiSaddr = pData08[i];
        }

    } else if (uiFormat == 2) {
        uiCount = uiLength / 2;
        pData16 = (UINT16 *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            *(volatile  UINT16  *)uiSaddr = pData16[i];
        }
    } else if (uiFormat == 4) {
        uiCount = uiLength / 4;
        pData32 = (UINT32 *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            *(volatile  UINT32  *)uiSaddr = pData32[i];
        }
    } else {
        memoryHelp();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memCopy
** 功能描述: 内存拷贝
** 输    入: uiDaddr      目的地址
**           uiSaddr      源地址
**           uiLength     长度
**           uiFormat     格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static INT  memCopy (UINT32  uiDaddr, UINT32  uiSaddr, UINT32  uiLength, UINT32  uiFormat)
{
    UINT32   i;
    UINT32   uiCount;

    volatile  UINT8  *pSrcData08;
    volatile  UINT16 *pSrcData16;
    volatile  UINT32 *pSrcData32;
    volatile  UINT8  *pDstData08;
    volatile  UINT16 *pDstData16;
    volatile  UINT32 *pDstData32;

    if (uiFormat == 1) {
        uiCount = uiLength;
        pSrcData08 = (UINT8  *)uiSaddr;
        pDstData08 = (UINT8  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pDstData08[i] = pSrcData08[i];
        }

    } else if (uiFormat == 2) {
        uiCount = uiLength / 2;
        pSrcData16 = (UINT16  *)uiSaddr;
        pDstData16 = (UINT16  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pDstData16[i] = pSrcData16[i];
        }
    } else if (uiFormat == 4) {
        uiCount = uiLength / 4;
        pSrcData32 = (UINT32  *)uiSaddr;
        pDstData32 = (UINT32  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pDstData32[i] = pSrcData32[i];
        }
    } else {
        memoryHelp();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memCompare
** 功能描述: 内存比较
** 输    入: uiDaddr      目的地址
**           uiSaddr      源地址
**           uiLength     长度
**           uiFormat     格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static INT  memCompare (UINT32  uiDaddr, UINT32  uiSaddr, UINT32  uiLength, UINT32  uiFormat)
{
    UINT32   i;
    UINT32   uiCount;
    UINT32   uiCountDif = 0;

    volatile  UINT8  *pSrcData08;
    volatile  UINT16 *pSrcData16;
    volatile  UINT32 *pSrcData32;
    volatile  UINT8  *pDstData08;
    volatile  UINT16 *pDstData16;
    volatile  UINT32 *pDstData32;

    if (uiFormat == 1) {
        uiCount = uiLength;
        pSrcData08 = (UINT8  *)uiSaddr;
        pDstData08 = (UINT8  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            if (pDstData08[i] != pSrcData08[i]) {
                uiCountDif++;
                PRINT_INFO("[%08x -- %08x] %02x -- %02x\n",
                            (UINT)&pDstData08[i], (UINT)&pSrcData08[i], pDstData08[i], pSrcData08[i]);
            }
        }
        PRINT_INFO("different count is %u\n", uiCountDif);
    } else if (uiFormat == 2) {
        uiCount = uiLength / 2;
        pSrcData16 = (UINT16  *)uiSaddr;
        pDstData16 = (UINT16  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            if (pDstData16[i] != pSrcData16[i]) {
                uiCountDif++;
                PRINT_INFO("[%08x -- %08x] %04x -- %04x\n",
                            (UINT)&pDstData16[i], (UINT)&pSrcData16[i], pDstData16[i], pSrcData16[i]);
            }
        }
        PRINT_INFO("different count is %u\n", uiCountDif);
    } else if (uiFormat == 4) {
        uiCount = uiLength;
        pSrcData32 = (UINT32  *)uiSaddr;
        pDstData32 = (UINT32  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            if (pDstData32[i] != pSrcData32[i]) {
                uiCountDif++;
                PRINT_INFO("[%08x -- %08x] %08x -- %08x\n",
                            (UINT)&pDstData32[i], (UINT)&pSrcData32[i], pDstData32[i], pSrcData32[i]);
            }
        }
        PRINT_INFO("different count is %u\n", uiCountDif);
    } else {
        memoryHelp();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memTest
** 功能描述: 内存可读写测试
** 输    入: uiDaddr      目的地址
**           uiLength     长度
**           uiFormat     格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static INT  memTest (UINT32  uiDaddr, UINT32  uiLength, UINT32  uiFormat)
{
    UINT32   i;
    UINT32   uiCount;

    volatile  UINT8  *pDstData08;
    volatile  UINT16 *pDstData16;
    volatile  UINT32 *pDstData32;

    if (uiFormat == 1) {
        uiCount = uiLength;
        pDstData08 = (UINT8  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pDstData08[i] = 0;
            if (pDstData08[i] != 0) {
                PRINT_INFO("test write 0x00 fault at %08x\n", (UINT)&pDstData08[i]);
            }
            pDstData08[i] = 0xff;
            if (pDstData08[i] != 0xff) {
                PRINT_INFO("test write 0xff fault at %08x\n", (UINT)&pDstData08[i]);
            }
        }

    } else if (uiFormat == 2) {
        uiCount = uiLength / 2;
        pDstData16 = (UINT16  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pDstData16[i] = 0;
            if (pDstData16[i] != 0) {
                PRINT_INFO("test write 0x0000 fault at %08x\n", (UINT)&pDstData16[i]);
            }
            pDstData16[i] = 0xffff;
            if (pDstData16[i] != 0xffff) {
                PRINT_INFO("test write 0xffff fault at %08x\n", (UINT)&pDstData16[i]);
            }
        }
    } else if (uiFormat == 4) {
        uiCount = uiLength / 4;
        pDstData32 = (UINT32  *)uiDaddr;
        for (i = 0; i < uiCount; i++) {
            pDstData32[i] = 0;
            if (pDstData32[i] != 0) {
                PRINT_INFO("test write 0x00000000 fault at %08x\n", (UINT)&pDstData32[i]);
            }
            pDstData32[i] = 0xffffffff;
            if (pDstData32[i] != 0xffffffff) {
                PRINT_INFO("test write 0xffffffff fault at %08x\n", (UINT)&pDstData32[i]);
            }
        }
    } else {
        memoryHelp();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memSpeed
** 功能描述: 内存读写速度测试
** 输    入: uiDaddr      目的地址
**           uiSaddr      源地址
**           uiLength     长度
**           uiFormat     格式
** 输    出: ERROR_CODE
*********************************************************************************************************/
static  INT  memSpeed (UINT32  uiDaddr, UINT32  uiLength, UINT32  uiFormat)
{
    addr_t   ulAddr;
    UINT32   i;
    UINT32   v;
    UINT32   uiCount;
    UINT64   t[2];
    UINT32   time[2];

    ulAddr  = uiDaddr;
    if (uiFormat == 1) {
        uiCount = uiLength / (16 * 1);
        MEMTEST(8);
    } else if (uiFormat == 2) {
        uiCount = uiLength / (16 * 2);
        MEMTEST(16);
    } else if (uiFormat == 4) {
        uiCount = uiLength / (16 * 4);
        MEMTEST(32);
    } else {
        memoryHelp();
        return  (PX_ERROR);
    }

    printf("memspeed read  %10d byte by %10u us (%7.2fMBps)\r\n",
           uiLength, time[0], uiLength * 1.0 / time[0]);
    printf("memspeed write %10d byte by %10u us (%7.2fMBps)\r\n",
           uiLength, time[1], uiLength * 1.0 / time[1]);

    return  (ERROR_NONE);
}
/*********************************************************************************************************
** 函数名称: memAccessCmd
** 功能描述: 内存访问命令
** 输    入: iArgC
**           ppcArgV
** 输    出: ERROR_CODE
*********************************************************************************************************/
INT  memAccessCmd (INT  iArgC, PCHAR  ppcArgV[])
{
    static ARG_ST  _S_args = {
        .cOperate  = 'r',
        .uiFormat  = 1,
        .uiSaddr   = 0,
        .uiDaddr   = 0,
        .uiValue   = 0,
        .uiLength  = 64,
    };
    ARG_ST  *pArg = &_S_args;

    if (strcmp(ppcArgV[1], "-i") == 0) {
        memoryInfo(pArg);
        return  (ERROR_NONE);
    }

    if (ERROR_NONE != memoryParseArg(pArg, iArgC, ppcArgV)) {
        memoryHelp();

        return  (PX_ERROR);
    }

    if (pArg->cOperate == 'r') {
        PRINT_INFO("memory read.\n");
        PRINT_INFO("uiSaddr = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiSaddr, pArg->uiLength, pArg->uiFormat);
        memRead (pArg->uiSaddr, pArg->uiLength, pArg->uiFormat);

    } else if (pArg->cOperate == 'w') {
        PRINT_INFO("memory write.\n");
        PRINT_INFO("uiDaddr = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiDaddr, pArg->uiLength, pArg->uiFormat);
        memWrite(pArg->uiDaddr, pArg->uiLength, pArg->uiBuf, pArg->uiFormat);
        memRead (pArg->uiDaddr, pArg->uiLength * pArg->uiFormat, pArg->uiFormat);
    } else if (pArg->cOperate == 's') {
        PRINT_INFO("memory set.\n");
        PRINT_INFO("uiAddr = %08x, uiValue = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiDaddr, pArg->uiValue, pArg->uiLength, pArg->uiFormat);
        memSet(pArg->uiDaddr, pArg->uiLength, pArg->uiValue, pArg->uiFormat);

    } else if (pArg->cOperate == 'g') {
        PRINT_INFO("memory get.\n");
        PRINT_INFO("uiDaddr = %08x, uiSaddr = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiDaddr, pArg->uiSaddr, pArg->uiLength, pArg->uiFormat);
        memGet(pArg->uiDaddr, pArg->uiLength, pArg->uiSaddr, pArg->uiFormat);

    } else if (pArg->cOperate == 'p') {
        PRINT_INFO("memory put.\n");
        PRINT_INFO("uiDaddr = %08x, uiSaddr = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiDaddr, pArg->uiSaddr, pArg->uiLength, pArg->uiFormat);
        memPut(pArg->uiDaddr, pArg->uiLength, pArg->uiSaddr, pArg->uiFormat);

    } else if (pArg->cOperate == 'c') {
        PRINT_INFO("memory copy.\n");
        PRINT_INFO("uiDaddr = %08x, uiSaddr = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiDaddr, pArg->uiSaddr, pArg->uiLength, pArg->uiFormat);
        memCopy(pArg->uiDaddr, pArg->uiSaddr, pArg->uiLength, pArg->uiFormat);

    } else if (pArg->cOperate == 'm') {
        PRINT_INFO("memory compare\n");
        PRINT_INFO("uiDaddr = %08x, uiSaddr = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiDaddr, pArg->uiSaddr, pArg->uiLength, pArg->uiFormat);
        memCompare(pArg->uiDaddr, pArg->uiSaddr, pArg->uiLength, pArg->uiFormat);

    } else if (pArg->cOperate == 't') {
        PRINT_INFO("memory access test\n");
        PRINT_INFO("uiDaddr = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiDaddr, pArg->uiLength, pArg->uiFormat);
        memTest(pArg->uiDaddr,pArg->uiLength, pArg->uiFormat);
    } else if (pArg->cOperate == 'e') {
        PRINT_INFO("memory speed test\n");
        PRINT_INFO("uiDaddr = %08x, uiLength = %08x, uiFormat = %d\n",
                    pArg->uiDaddr, pArg->uiLength, pArg->uiFormat);
        memSpeed(pArg->uiDaddr,pArg->uiLength, pArg->uiFormat);
    } else {
        memoryHelp();
    }

    return  (ERROR_NONE);
}
/*********************************************************************************************************
  END
*********************************************************************************************************/
