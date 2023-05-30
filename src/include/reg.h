#pragma once

#include "types.h"

void reg_w_mscratch(reg_t x);
void reg_w_mtvec(reg_t x);

reg_t reg_r_mhartid();
reg_t reg_r_tp();

/* 机器模式下的状态寄存器标志, mstatus */
#define MSTATUS_MPP (3 << 11)
#define MSTATUS_MPIE (1 << 7)
#define MSTATUS_MIE (1 << 3)

reg_t reg_r_mstatus();
void reg_w_mstatus(reg_t x);

/* 机器模式下的中断使能 */
#define MIE_MEIE (1 << 11)  // external 外部中断
#define MIE_MTIE (1 << 7)   // timer 定时器中断
#define MIE_MSIE (1 << 3)   // software 软件中断

reg_t reg_r_mie();
void reg_w_mie(reg_t x);
