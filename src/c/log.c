
#include "log.h"

#include "printf.h"
#include "reg.h"

#define MAX_CALLBACKS 32

static struct {
  int level;
  bool quiet;
} L;

void log_set_level(int level) { L.level = level; }
void log_set_quiet(bool enable) { L.quiet = enable; }

static const char *level_colors[] = {"\x1b[94m", "\x1b[36m", "\x1b[32m",
                                     "\x1b[33m", "\x1b[31m", "\x1b[35m"};

static const char *level_strings[] = {"TRACE", "DEBUG", "INFO",
                                      "WARN",  "ERROR", "FATAL"};

void log_log(enum level level, const char *file, int line, const char *fmt,
             ...) {
  va_list ap;
  if (L.quiet || level < L.level) {
    return;
  }
  va_start(ap, fmt);

  // 关中断防止任务切换，以实现线程安全的log
  reg_w_mstatus(reg_r_mstatus() & ~MSTATUS_MIE);
  printf("%s[%s]\x1b[0m    \t\x1b[90m%s:%d:    \x1b[0m \t",  //
         level_colors[level],                                //
         level_strings[level],                               //
         file,                                               //
         line);
  vprintf(fmt, ap);
  printf("\n");
  if (level == LOG_FATAL) {
    while (1)
      ;
  }

  // 开中断
  reg_w_mstatus(reg_r_mstatus() | MSTATUS_MIE);
  va_end(ap);
}

const char *log_level_string(enum level level) { return level_strings[level]; }

enum level get_log_level() { return L.level; }