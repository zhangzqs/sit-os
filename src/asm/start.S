#include "platform.h"
    .equ    STACK_SIZE, 81920
    .global _start
    .text

_start:
    # 读取 t0 = mhardid
    # mhartid = machine hart id
    csrr    t0, mhartid

    # thread pointer 寄存器 tp = t0
    mv tp, t0

    // if (t0 != 0) jump to park
    bnez    t0, park

	# Set all bytes in the BSS section to zero.
	la	a0, _bss_start
	la	a1, _bss_end
	bgeu	a0, a1, 2f
1:
	sw	zero, (a0)
	addi	a0, a0, 4
	bltu	a0, a1, 1b
2:
    # 初始化栈，栈的增长方向为从低地址到高地址
    # 将栈指针指向栈边界的末尾
    # t0 = t0 << 10
    slli    t0, t0, 10

    # sp = stacks + STACK_SIZE
    la  sp, stacks + STACK_SIZE
    
    # sp = sp + t0
    add sp, sp, t0


	# At the end of start_kernel, schedule() will call MRET to switch
	# to the first task, so we parepare the mstatus here.
	# Notice: default mstatus is 0
	# Set mstatus.MPP to 3, so we still run in Machine mode after MRET.
	# Set mstatus.MPIE to 1, so MRET will enable the interrupt.
	li	t0, 3 << 11 | 1 << 7
	csrr	a1, mstatus
	or	t0, t0, a1
	csrw	mstatus, t0

    # 转至c语言main函数
    j   start_kernel

park:
    # 使处理器进入空闲状态 Wait for interrupt
    wfi
    j   park

stacks:
    .skip   STACK_SIZE * MAXNUM_CPU
    .end

