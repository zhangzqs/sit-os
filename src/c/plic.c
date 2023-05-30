#include "platform.h"
#include "reg.h"
void plic_init() {
  int hart = reg_r_tp();
  // 设置 UART0 优先级
  *(uint32_t*)PLIC_PRIORITY(UART0_IRQ) = 1;

  // 启用 UART0 中断
  *(uint32_t*)PLIC_MENABLE(hart) = (1 << UART0_IRQ);

  // 设置 UART0 的 threshold
  *(uint32_t*)PLIC_MTHRESHOLD(hart) = 0;

  // 启用机器模式下的外部中断
  reg_w_mie(reg_r_mie() | MIE_MEIE);
}

int plic_claim() {
  int hart = reg_r_tp();
  int irq = *(uint32_t*)PLIC_MCLAIM(hart);
  return irq;
}

void plic_complete(int irq) {
  int hart = reg_r_tp();
  *(uint32_t*)PLIC_MCOMPLETE(hart) = irq;
}