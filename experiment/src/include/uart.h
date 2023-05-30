#pragma once

#include "types.h"

class UART {
 private:
  uint8_t *_base;

  inline uint8_t read_reg(uint8_t reg) { return *(_base + reg); }
  inline void write_reg(uint8_t reg, uint8_t v) { *(_base + reg) = v; }

 public:
  explicit UART(uint8_t *base);
  void putc(char ch);
};