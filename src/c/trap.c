#include "trap.h"

#include "log.h"
#include "platform.h"
#include "printf.h"
#include "reg.h"
#include "timer.h"
#include "types.h"
extern void trap_vector();

// 外部中断
static void external_interrupt_handler() {
  int irq = plic_claim();

  if (irq == UART0_IRQ) {
    uart_isr();
  } else if (irq) {
    printf("unexpected interrupt irq = %d\n", irq);
  }

  if (irq) {
    plic_complete(irq);
  }
}

void trap_init() {
  // 设置机器模式下的trap向量基地址
  // 当trap发送时，pc会被替换成mtvec设定的地址
  // 同时hart会设置mepc为当前指令或下一条指令的地址
  // 当我们需要退出trap时，调用mret指令，则会将mepc的值恢复到pc中，实现恢复中断前到现场

  reg_w_mtvec((reg_t)trap_vector);
}

// 从汇编跳转到c语言的trap处理函数
// epc为mepc寄存器，cause为mcause寄存器的值
// 返回值将被设置到mepc寄存器中
reg_t trap_handler(reg_t epc, reg_t cause) {
  // log_trace("trap_handler called!!!");
  reg_t return_pc = epc;
  reg_t cause_code = cause & 0xfff;

  // mcause寄存器到最高位interrupt为1表示当前trap为interrupt，否则是exception
  if (cause & 0x80000000) {
    /* Asynchronous trap - interrupt */
    switch (cause_code) {
      case 3:
        log_trace("software interruption!");
        int id = reg_r_mhartid();
        *(uint32_t *)CLINT_MSIP(id) = 0;
        schedule();
        break;
      case 7:
        // log_trace("timer interruption!");
        timer_handler();
        break;
      case 11:
        // log_trace("external interruption!");
        external_interrupt_handler();
        break;
      default:
        log_trace("unknown async exception!");
        break;
    }
  } else {
    /* Synchronous trap - exception */
    // log_error("Sync exceptions!, code = %d", cause_code);
    // panic("OOPS! What can I do!");  // 发送了异常，不去处理
    // return_pc += 4;
  }

  return return_pc;
}

void trap_test() {
  /*
   * Synchronous exception code = 7
   * Store/AMO access fault
   */
  *(int *)0x00000000 = 100;

  /*
   * Synchronous exception code = 5
   * Load access fault
   */
  // int a = *(int *)0x00000000;

  uart_puts("Yeah! I'm return back from trap!");
}
