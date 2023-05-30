#include "fmt.h"
#include "platform.h"
#include "uart.h"

extern "C" void start_kernel() {
  UART uart((uint8_t *)UART0);
  Formatter fmt(&uart);
  fmt.printf("HelloWorld %d %x %p\n", 123, 'a', 0x1234, &fmt);
}
