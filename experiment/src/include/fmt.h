#pragma once

#include <stdarg.h>

#include "types.h"
#include "uart.h"

/// 针对字符设备格式化输出的类
class Formatter {
 private:
  UART *_uart;

  void _printint(int xx, int base, int sign);
  void _printptr(uint32_t x);

 public:
  explicit Formatter(UART *uart);

  // 格式化打印到指定的字符设备上暂时只支持 %d, %x, %p, %s.
  void vprintf(char *fmt, va_list ap);
  void printf(char *fmt, ...);
  void panic(char *s);
};