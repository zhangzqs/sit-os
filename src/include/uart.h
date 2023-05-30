#pragma once

void uart_init();
void uart_puts(char *s);
void uart_putc(char ch);
char *uart_getline();
char uart_getch();
char uart_getch_nowait();