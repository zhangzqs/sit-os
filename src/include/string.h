#pragma once
#include "types.h"

void memset(void *dest, char val, int count);
void memcpy(void *dest, const void *src, int count);
int memcmp(const void *s1, const void *s2, int count);
char *strchr(const char *s, char c);
int strlen(const char *s);
int atoi(const char *s);
char *itoa(int num, char *str, int radix);
uint32_t hex2uint32(const char *s);
int strcmp(const char *s1, const char *s2);
char *strtok(char *s, const char *delim);
char *strcat(char *dest, const char *src);
char *strrchr(const char *s, char c);
char *strcpy(char *dest, const char *src);
char *strtok_with_ch_mem(char **p_s_mem, char *str, const char *delimiters);
char toupper(char c);
bool startswith(const char *s, const char *prefix);
bool endswith(const char *s, const char *suffix);