#include "log.h"
#include "platform.h"
#include "reg.h"
#include "types.h"
/* interval ~= 1s */
#define TIMER_INTERVAL CLINT_TIMEBASE_FREQ / 10

static uint32_t _tick = 0;

/* load timer interval(in ticks) for next timer interrupt.*/
void timer_load(int interval) {
  /* each CPU has a separate source of timer interrupts. */
  int id = reg_r_mhartid();

  *(uint64_t*)CLINT_MTIMECMP(id) = *(uint64_t*)CLINT_MTIME + interval;
}

void timer_init() {
  /*
   * On reset, mtime is cleared to zero, but the mtimecmp registers
   * are not reset. So we have to init the mtimecmp manually.
   */
  timer_load(TIMER_INTERVAL);

  /* enable machine-mode timer interrupts. */
  reg_w_mie(reg_r_mie() | MIE_MTIE);
}

void timer_handler() {
  _tick++;
  // log_trace("tick: %d", _tick);

  timer_load(TIMER_INTERVAL);

  schedule();
}
