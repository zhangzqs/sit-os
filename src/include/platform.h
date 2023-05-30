#pragma once

#define MAXNUM_CPU 8

/*
 * MemoryMap
 * see https://github.com/qemu/qemu/blob/master/hw/riscv/virt.c, virt_memmap[]
 * 0x00001000 -- boot ROM, provided by qemu 引导ROM
 * 0x02000000 -- CLINT
 * 0x0C000000 -- PLIC 平台级中断控制器
 * 0x10000000 -- UART0 串口
 * 0x10001000 -- virtio disk 磁盘设备
 * 0x80000000 -- 引导ROM将跳转到该地址处的代码进行执行，这也是我们的内核入口代码地址
 */

// UART0的地址起始位置
#define UART0 0x10000000L

// UART0的中断源
#define UART0_IRQ 10

// 机器模式下的PLIC相关寄存器的地址
#define PLIC_BASE 0x0C000000L
#define PLIC_PRIORITY(id) (PLIC_BASE + (id)*4)
#define PLIC_PENDING(id) (PLIC_BASE + 0x1000 + ((id) / 32) * 4)
#define PLIC_MENABLE(hart) (PLIC_BASE + 0x2000 + (hart)*0x80)
#define PLIC_MTHRESHOLD(hart) (PLIC_BASE + 0x200000 + (hart)*0x1000)
#define PLIC_MCLAIM(hart) (PLIC_BASE + 0x200004 + (hart)*0x1000)
#define PLIC_MCOMPLETE(hart) (PLIC_BASE + 0x200004 + (hart)*0x1000)

#define CLINT_BASE 0x2000000L
#define CLINT_MSIP(hartid) (CLINT_BASE + 4 * (hartid))
#define CLINT_MTIMECMP(hartid) (CLINT_BASE + 0x4000 + 8 * (hartid))
#define CLINT_MTIME (CLINT_BASE + 0xBFF8)  // cycles since boot.

// 时钟频率
#define CLINT_TIMEBASE_FREQ 10000000
