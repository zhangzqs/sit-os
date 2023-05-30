
#include "printf.h"

#include <stdarg.h>

#include "log.h"
#include "types.h"
#include "uart.h"

static char digits[] = "0123456789abcdef";

static char *p_str = NULL;

static void putc(char c) {
  if (p_str != NULL) {
    *p_str++ = c;
    return;
  }
  uart_putc(c);
}

static void printint(int xx, int base, int sign) {
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

  while (--i >= 0) putc(buf[i]);
}

static void printptr(uint32_t x) {
  int i;
  putc('0');
  putc('x');
  for (i = 0; i < (sizeof(uint32_t) * 2); i++, x <<= 4) putc(digits[x >> (sizeof(uint32_t) * 8 - 4)]);
}

// Print to the console. only understands %d, %x, %p, %s.
void vprintf(char *fmt, va_list ap) {
  int i, c;
  char *s;

  if (fmt == 0) panic("null fmt");

  for (i = 0; (c = fmt[i] & 0xff) != 0; i++) {
    if (c != '%') {
      putc(c);
      continue;
    }
    c = fmt[++i] & 0xff;
    if (c == 0) break;
    switch (c) {
      case 'd':
        printint(va_arg(ap, int), 10, 1);
        break;
      case 'c':
        putc(va_arg(ap, char *));
        break;
      case 'x':
        printint(va_arg(ap, int), 16, 1);
        break;
      case 'p':
        printptr(va_arg(ap, uint32_t));
        break;
      case 's':
        if ((s = va_arg(ap, char *)) == 0) s = "(null)";
        for (; *s; s++) putc(*s);
        break;
      case '%':
        putc('%');
        break;
      default:
        // Print unknown % sequence to draw attention.
        putc('%');
        putc(c);
        break;
    }
  }
}

void printf(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
}

void sprintf(char *buf, char *fmt, ...) {
  p_str = buf;
  va_list ap;
  va_start(ap, fmt);
  vprintf(fmt, ap);
  va_end(ap);
  *p_str = '\0';
  p_str = NULL;
}

void redirect_printf(char *buf) { p_str = buf; }
void restore_printf() { p_str = NULL; }

void clear() {
    printf("\x1b[2J\x1b[H");
}

void panic(char *s) {
  log_error("kernel panic: %s", s);
  while (1) {
  }
}
