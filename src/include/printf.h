#pragma once

void printf(char *fmt, ...);
void panic(char *s);
void sprintf(char *buf, char *fmt, ...);
void redirect_printf(char *buf);
void restore_printf();
void clear();