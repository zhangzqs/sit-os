#include "log.h"
#include "printf.h"
#include "task.h"
#include "timer.h"
#include "trap.h"
#include "uart.h"
#include "user.h"

void start_kernel() {
  uart_init();

  log_set_level(LOG_DEBUG);
  log_set_quiet(0);

  log_info("Kernel start!!!");
  heap_init();

  task_init();
  user_main();
  trap_init();
  plic_init();
  timer_init();
  schedule();

  while (1) {
    log_debug("no user task!!!");
    task_delay(10);
  }
}