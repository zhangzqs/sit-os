#include "types.h"

// machine模式下专业的寄存器，可自定义其用法，如可保存在hart上运行的task上下文context
void reg_w_mscratch(reg_t x) { asm volatile("csrw mscratch, %0" : : "r"(x)); }

// 机器模式下的中断向量，发生异常时处理器需要跳转到的地址
// Machine Trap-Vector Base-Address
// 低2位为MODE，其余30位为入口函数的BASE地址，故将保证四字节对齐
void reg_w_mtvec(reg_t x) { asm volatile("csrw mtvec, %0" : : "r"(x)); }

/* which hart (core) is this? */
reg_t reg_r_mhartid() {
  reg_t x;
  asm volatile("csrr %0, mhartid" : "=r"(x));
  return x;
}

// 读取当前hartid
reg_t reg_r_tp() {
  reg_t x;
  asm volatile("mv %0, tp" : "=r"(x));
  return x;
}

// 读取当前mstatus
reg_t reg_r_mstatus() {
  reg_t x;
  asm volatile("csrr %0, mstatus" : "=r"(x));
  return x;
}

// 写入当前mstatus
void reg_w_mstatus(reg_t x) { asm volatile("csrw mstatus, %0" : : "r"(x)); }

// 读取mie
reg_t reg_r_mie() {
  reg_t x;
  asm volatile("csrr %0, mie" : "=r"(x));
  return x;
}

// 写入mie
void reg_w_mie(reg_t x) { asm volatile("csrw mie, %0" : : "r"(x)); }

// /*
//  * ref: https://github.com/mit-pdos/xv6-riscv/blob/riscv/kernel/riscv.h
//  */

// /*
//  * machine exception program counter, holds the
//  * instruction address to which a return from
//  * exception will go.
//  */
// static inline void w_mepc(reg_t x)
// {
// 	asm volatile("csrw mepc, %0" : : "r" (x));
// }

// static inline reg_t r_mepc()
// {
// 	reg_t x;
// 	asm volatile("csrr %0, mepc" : "=r" (x));
// 	return x;
// }

// /* Machine Scratch register, for early trap handler */
// static inline void w_mscratch(reg_t x)
// {
// 	asm volatile("csrw mscratch, %0" : : "r" (x));
// }

// static inline reg_t r_mcause()
// {
// 	reg_t x;
// 	asm volatile("csrr %0, mcause" : "=r" (x) );
// 	return x;
// }
