#include "string.h"

// 设置内存为val
void memset(void *dest, char val, int count) {
  unsigned char *temp = (unsigned char *)dest;
  for (; count != 0; count--) *temp++ = val;
}

// 拷贝内存
void memcpy(void *dest, const void *src, int count) {
  char *dst8 = (char *)dest;
  const char *src8 = (const char *)src;
  for (; count != 0; count--) *dst8++ = *src8++;
}

// 比较两个内存，返回0表示相等
int memcmp(const void *s1, const void *s2, int count) {
  const unsigned char *su1, *su2;
  int res = 0;
  for (su1 = s1, su2 = s2; 0 < count; ++su1, ++su2, count--)
    if ((res = *su1 - *su2) != 0) break;
  return res;
}

// 寻找到第一个匹配的字符，返回指针
// 如果没有找到，返回0
char *strchr(const char *s, char c) {
  for (; *s != (char)c; ++s)
    if (*s == '\0') return 0;
  return (char *)s;
}

// 字符串长度
int strlen(const char *s) {
  int n;
  for (n = 0; *s != '\0'; s++) n++;
  return n;
}

// 字符串拷贝
char *strcpy(char *dest, const char *src) {
  char *tmp = dest;
  while ((*dest++ = *src++) != '\0')
    ;
  return tmp;
}

// 字符串转化为数字
int atoi(const char *s) {
  int i = 0;
  while (*s >= '0' && *s <= '9') {
    i = i * 10 + (*s - '0');
    s++;
  }
  return i;
}

// 数字转化为字符串形式
char *itoa(int num, char *str, int radix) {
  char index[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  unsigned unum;
  int i = 0, j, k;
  if (radix == 10 && num < 0) {
    unum = (unsigned)-num;
    str[i++] = '-';
  } else
    unum = (unsigned)num;
  do {
    str[i++] = index[unum % (unsigned)radix];
    unum /= radix;
  } while (unum);
  str[i] = '\0';
  if (str[0] == '-')
    k = 1;
  else
    k = 0;
  char temp;
  for (j = k; j <= (i - 1) / 2; j++) {
    temp = str[j];
    str[j] = str[i - 1 + k - j];
    str[i - 1 + k - j] = temp;
  }
  return str;
}

uint32_t hex2uint32(const char *s) {
  uint32_t ret = 0;
  while (*s) {
    ret <<= 4;
    if (*s >= '0' && *s <= '9') {
      ret += *s - '0';
    } else if (*s >= 'a' && *s <= 'f') {
      ret += *s - 'a' + 10;
    } else if (*s >= 'A' && *s <= 'F') {
      ret += *s - 'A' + 10;
    } else {
      break;
    }
    s++;
  }
  return ret;
}

// 字符串比较
int strcmp(const char *s1, const char *s2) {
  while (*s1 && *s1 == *s2) {
    s1++;
    s2++;
  }
  return *s1 - *s2;
}

char *strtok(char *str, const char *delimiters) {
  static char *s_mem = NULL;
  strtok_with_ch_mem(&s_mem, str, delimiters);
}

// 字符串分割
char *strtok_with_ch_mem(char **p_s_mem, char *str, const char *delimiters) {
  if (delimiters == NULL) return NULL;
  char *s_mem = *p_s_mem;
  if (str == NULL && s_mem == NULL) return NULL;

  char *s;
  if (str != NULL)
    s = str;
  else
    s = s_mem;  // 在str ==
                // NULL的情况下，s_mem一定不等于NULL，因为上面的那个if判断
  char const *delim;

  // 找到token的起始位置
  int stat = 1;
  while (stat) {
    delim = delimiters;  // 一开始没写这句，这里不赋初值怎么在stat里面使用！！！
    while (*delim && *s != *delim) {
      delim++;
    }
    // 因为delim绝对不会到 /0 ，所以s会在到达 /0 时跳出，
    if (*delim) {  // 若匹配上
      s++;
    } else
      stat = 0;
  }
  s_mem = s;  // 此时 *s 不为delimiters
  *p_s_mem = s_mem;
  // 找结尾
  while (*s) {
    delim = delimiters;
    while (*delim && *s != *delim) {
      delim++;
    }
    // 跳出情况两种：
    // 1.*delim为 /0 ，此时说明 *s 与 delimiters没有一样，换下一个 *s
    // 2.*delim不为 /0 的情况下，*s与*delim匹配上，说明在此处应该断开
    if (*delim) {
      *s = '\0';
      char *t = s_mem;
      s_mem = s + 1;
      *p_s_mem = s_mem;
      return t;
    }
    s++;
  }
  // 跳出时，说明 s 中没有delimiters
  char *t = s_mem;
  s_mem = NULL;
  *p_s_mem = s_mem;
  return t;
}

// 字符串拼接
char *strcat(char *dest, const char *src) {
  char *tmp = dest;
  while (*dest) dest++;
  while ((*dest++ = *src++) != '\0')
    ;
  return tmp;
}

// str 所指向的字符串中搜索最后一次出现字符 c
char *strrchr(const char *s, char c) {
  const char *last = NULL;
  do {
    if (*s == c) last = s;
  } while (*s++);
  return (char *)last;
}

char toupper(char c) {
  if (c >= 'a' && c <= 'z') {
    return c - 'a' + 'A';
  }
  return c;
}

bool startswith(const char *s, const char *prefix) {
  while (*prefix) {
    if (*s++ != *prefix++) return false;
  }
  return true;
}

bool endswith(const char *s, const char *suffix) {
  int len_s = strlen(s);
  int len_suffix = strlen(suffix);
  if (len_s < len_suffix) return false;
  s += len_s - len_suffix;
  while (*suffix) {
    if (*s++ != *suffix++) return false;
  }
  return true;
}