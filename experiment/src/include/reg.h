#include "types.h"

/* 机器模式下的状态寄存器标志, mstatus */
#define MSTATUS_MPP (3 << 11)
#define MSTATUS_MPIE (1 << 7)
#define MSTATUS_MIE (1 << 3)

/* 机器模式下的中断使能 */
#define MIE_MEIE (1 << 11)  // external 外部中断
#define MIE_MTIE (1 << 7)   // timer 定时器中断
#define MIE_MSIE (1 << 3)   // software 软件中断

namespace Reg {

// machine模式下专用的寄存器，可自定义其用法，如可保存在hart上运行的task上下文context
void write_mscratch(reg_t x) { asm volatile("csrw mscratch, %0" : : "r"(x)); }

// 机器模式下的中断向量，发生异常时处理器需要跳转到的地址
// Machine Trap-Vector Base-Address
// 低2位为MODE，其余30位为入口函数的BASE地址，故将保证四字节对齐
void write_mtvec(reg_t x) { asm volatile("csrw mtvec, %0" : : "r"(x)); }

/* which hart (core) is this? */
reg_t read_mhartid() {
  reg_t x;
  asm volatile("csrr %0, mhartid" : "=r"(x));
  return x;
}

// 读取当前hartid
reg_t read_tp() {
  reg_t x;
  asm volatile("mv %0, tp" : "=r"(x));
  return x;
}

// 读取当前mstatus
reg_t read_mstatus() {
  reg_t x;
  asm volatile("csrr %0, mstatus" : "=r"(x));
  return x;
}

// 写入当前mstatus
void write_mstatus(reg_t x) { asm volatile("csrw mstatus, %0" : : "r"(x)); }

// 读取mie
reg_t read_mie() {
  reg_t x;
  asm volatile("csrr %0, mie" : "=r"(x));
  return x;
}

// 写入mie
void write_mie(reg_t x) { asm volatile("csrw mie, %0" : : "r"(x)); }

};  // namespace Reg