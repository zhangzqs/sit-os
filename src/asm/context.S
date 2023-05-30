.macro reg_save base
	sw ra, 0(\base)
	sw sp, 4(\base)
	sw gp, 8(\base)
	sw tp, 12(\base)
	sw t0, 16(\base)
	sw t1, 20(\base)
	sw t2, 24(\base)
	sw s0, 28(\base)
	sw s1, 32(\base)
	sw a0, 36(\base)
	sw a1, 40(\base)
	sw a2, 44(\base)
	sw a3, 48(\base)
	sw a4, 52(\base)
	sw a5, 56(\base)
	sw a6, 60(\base)
	sw a7, 64(\base)
	sw s2, 68(\base)
	sw s3, 72(\base)
	sw s4, 76(\base)
	sw s5, 80(\base)
	sw s6, 84(\base)
	sw s7, 88(\base)
	sw s8, 92(\base)
	sw s9, 96(\base)
	sw s10, 100(\base)
	sw s11, 104(\base)
	sw t3, 108(\base)
	sw t4, 112(\base)
	sw t5, 116(\base)
	# sw t6, 120(\base)
	# 由于我们使用t6本身作为基地址，故我们不能在此处保存t6点值，需要在外部编写一些额外的代码
.endm

.macro reg_restore base
	lw ra, 0(\base)
	lw sp, 4(\base)
	lw gp, 8(\base)
	lw tp, 12(\base)
	lw t0, 16(\base)
	lw t1, 20(\base)
	lw t2, 24(\base)
	lw s0, 28(\base)
	lw s1, 32(\base)
	lw a0, 36(\base)
	lw a1, 40(\base)
	lw a2, 44(\base)
	lw a3, 48(\base)
	lw a4, 52(\base)
	lw a5, 56(\base)
	lw a6, 60(\base)
	lw a7, 64(\base)
	lw s2, 68(\base)
	lw s3, 72(\base)
	lw s4, 76(\base)
	lw s5, 80(\base)
	lw s6, 84(\base)
	lw s7, 88(\base)
	lw s8, 92(\base)
	lw s9, 96(\base)
	lw s10, 100(\base)
	lw s11, 104(\base)
	lw t3, 108(\base)
	lw t4, 112(\base)
	lw t5, 116(\base)
	lw t6, 120(\base)
.endm

.text

# 机器模式下的中断和异常处理点
.globl trap_vector
# 中断向量地址必须要4字节对齐
.align 4
trap_vector:
	# 保存上下文到基地址 t6
	csrrw	t6, mscratch, t6 # swap
	reg_save t6

	# 现在寄存器现场已经被保存, 可以随意使用寄存器了
	# 把上下文地址存到 t5 = t6
	mv	t5, t6

	# 恢复原来的t6 = mscratch
	csrr	t6, mscratch

	# 把t6写到上下文里 *(t5 + 120) = t6
	sw	t6, 120(t5)	

	# 保存mepc的值到context结构体的pc变量中
	csrr	a0, mepc
	sw	a0, 124(t5)


	# 再把上下文恢复回mscratch，mscratch = t5
	csrw	mscratch, t5

	# 此时寄存器现场已经完全保存了下来

	# 调用c语言中的trap_handler函数
	csrr	a0, mepc
	csrr	a1, mcause
	call	trap_handler

	# 将trap_handler函数的返回值设置到 mepc = trap_handler(mepc, mcause)
	csrw	mepc, a0

	# 恢复现场
	csrr	t6, mscratch
	reg_restore t6

	# 处理完毕后返回到mepc处
	mret

# 切换到的目标任务上下文指针
# a0: 切换到目标任务 
.globl switch_to
.align 4
switch_to:
	# 利用mscratch寄存器暂存寄存器a0的值
	csrw	mscratch, a0

	# 设置mepc寄存器的值为next任务上下文中的pc变量
	lw	a1, 124(a0)
	csrw	mepc, a1

	# 以t6为中间变量作为context的基地址恢复所有寄存器的值
	mv	t6, a0
	reg_restore t6

	# 触发软件中断，使CPU回到mepc指向的程序地址
	mret
.end

