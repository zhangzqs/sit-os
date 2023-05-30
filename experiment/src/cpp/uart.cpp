#include "uart.h"

// UART 芯片的8个寄存器
#define RHR 0  // Receive Holding Register (read mode)
#define THR 0  // Transmit Holding Register (write mode)
#define DLL 0  // LSB of Divisor Latch (write mode)
#define IER 1  // Interrupt Enable Register (write mode)
#define DLM 1  // MSB of Divisor Latch (write mode)
#define FCR 2  // FIFO Control Register (write mode)
#define ISR 2  // Interrupt Status Register (read mode)
#define LCR 3  // Line Control Register
#define MCR 4  // Modem Control Register
#define LSR 5  // Line Status Register
#define MSR 6  // Modem Status Register
#define SPR 7  // ScratchPad Register
#define LSR_RX_READY (1 << 0)

UART::UART(uint8_t *base) : _base(base) {
  // 禁用uart芯片所有中断信号
  write_reg(IER, 0x00);

  // 设置LCR寄存器第7位为1才能够进入设置DL寄存器状态
  uint8_t lcr = read_reg(LCR);
  // Divisor latch enable
  write_reg(LCR, lcr | (1 << 7));

  // Divisor Latch 寄存器低位0x03
  write_reg(DLL, 0x03);
  // Divisor Latch 寄存器高位0x00
  write_reg(DLM, 0x00);

  // BIT-1    BIT-0 WORD LENGTH
  //  0         0           5
  //  0         1           6
  //  1         0           7
  //  1         1           8
  // bit2 停止位 = 0
  // bit3 不使用奇偶校验
  // bit4 奇校验为1 偶校验为0
  // bit7 divisor latch enable
  write_reg(LCR, 0b00000011);

  // 启用串口接收中断
  uint8_t ier = read_reg(IER);
  write_reg(IER, ier | (1 << 0));
}

void UART::putc(char ch) {
  uint8_t lsr = read_reg(LSR);

  // 判断串口输出是否空闲
  bool idle;
  do {
    // 只要LSR的bit5为1则表示THR寄存器空闲，随后写入
    idle = (lsr >> 5) & 1;
  } while (!idle);  // 只要不空闲，就再循环一次，直到空闲
  write_reg(THR, ch);
}