#include "fmt.h"

#include "uart.h"

static char *digits = "0123456789abcdef";

Formatter::Formatter(UART *uart) : _uart(uart) {}

void Formatter::_printint(int xx, int base, int sign) {
  char buf[16];
  int i;
  uint32_t x;

  if (sign && (sign = xx < 0))
    x = -xx;
  else
    x = xx;

  i = 0;
  do {
    buf[i++] = digits[x % base];
  } while ((x /= base) != 0);

  if (sign) buf[i++] = '-';

  while (--i >= 0) _uart->putc(buf[i]);
}

void Formatter::_printptr(uint32_t x) {
  int i;
  _uart->putc('0');
  _uart->putc('x');
  for (i = 0; i < (sizeof(uint32_t) * 2); i++, x <<= 4) {
    _uart->putc(digits[x >> (sizeof(uint32_t) * 8 - 4)]);
  }
}

void Formatter::vprintf(char *fmt, va_list ap) {
  int i, c;
  char *s;

  if (fmt == 0) panic("null fmt");

  for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
    if (c != '%') {
      _uart->putc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0) break;
    switch (c) {
      case 'd':
        _printint(va_arg(ap, int), 10, 1);
        break;
      case 'x':
        _printint(va_arg(ap, int), 16, 1);
        break;
      case 'p':
        _printptr(va_arg(ap, uint32_t));
        break;
      case 's':
        if ((s = va_arg(ap, char *)) == 0) s = "(null)";
        for (; *s; s++) _uart->putc(*s);
        break;
      case '%':
        _uart->putc('%');
        break;
      default:
        // Print unknown % sequence to draw attention.
        _uart->putc('%');
        _uart->putc(c);
        break;
    }
  }
}

void Formatter::printf(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

void Formatter::panic(char *s) {
  printf("kernel panic: %s", s);
  while (1) {
  }
}