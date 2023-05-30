#include "log.h"
#include "platform.h"
#include "types.h"
// 根据uart设备的相对地址计算出绝对地址下的内存映射
#define UART_REG(reg) ((volatile uint8_t *)(UART0 + reg))

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
#define LSR_TX_IDLE (1 << 5)

#define uart_read_reg(reg) (*(UART_REG(reg)))
#define uart_write_reg(reg, v) (*(UART_REG(reg)) = (v))

void uart_init() {
  // 禁用uart芯片所有中断信号
  uart_write_reg(IER, 0x00);

  // 设置LCR寄存器第7位为1才能够进入设置DL寄存器状态
  uint8_t lcr = uart_read_reg(LCR);
  // Divisor latch enable
  uart_write_reg(LCR, lcr | (1 << 7));

  // Divisor Latch 寄存器低位0x03
  uart_write_reg(DLL, 0x03);
  // Divisor Latch 寄存器高位0x00
  uart_write_reg(DLM, 0x00);

  // BIT-1    BIT-0 WORD LENGTH
  //  0         0           5
  //  0         1           6
  //  1         0           7
  //  1         1           8
  // bit2 停止位 = 0
  // bit3 不使用奇偶校验
  // bit4 奇校验为1 偶校验为0
  // bit7 divisor latch enable
  uart_write_reg(LCR, 0b00000011);

  // 启用串口接收中断
  uint8_t ier = uart_read_reg(IER);
  uart_write_reg(IER, ier | (1 << 0));
}

int uart_putc(char ch) {
  // LSR[5] transmit holding empty
  // 等待LSR寄存器bit5为1则表示THR寄存器空闲
  while ((uart_read_reg(LSR) & LSR_TX_IDLE) == 0)
    ;
  // THR transmit hold register
  // 传送数据使用的寄存器
  return uart_write_reg(THR, ch);
}

void uart_puts(char *s) {
  while (*s) uart_putc(*s++);
}

static int uart_getc() {
  if (uart_read_reg(LSR) & LSR_RX_READY) {
    return uart_read_reg(RHR);
  }
  return -1;
}

static bool getch_latch = false;
static unsigned char ch;

char uart_getch_nowait() {
  if (getch_latch) {
    getch_latch = false;
    return ch;
  }
  return -1;
}

// 读取一个字符
char uart_getch() {
  // 等待串口接收到一个字符
  while (true) {
    if (getch_latch) {
      getch_latch = false;
      return ch;
    }
  }
}

static char _buf[1024];
static int _buf_len = 0;
char *uart_getline() {
  // log_debug("getline");
  while (1) {
    char c = uart_getch();
    uart_putc(c);
    if (c == '\r') {
      // 回车键
      uart_putc('\n');
      _buf[_buf_len] = '\0';
      _buf_len = 0;
      return _buf;
    } else if (c == 0x7f) {
      // 回退键
      if (_buf_len > 0) {
        _buf_len--;
        printf("\b \b");
      }
    } else {
      // 正常打印字符
      _buf[_buf_len++] = c;
    }
  }
}
// 串口中断处理函数，将被trap.c中的中断服务函数所调用
void uart_isr() {
  while (1) {
    int c = uart_getc();
    if (c == -1) return;
    getch_latch = true;
    ch = c;
  }
}
