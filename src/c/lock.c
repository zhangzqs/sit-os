#include "lock.h"

#include <stdbool.h>
void lock_init(struct spinlock *lk) { lk->locked = 0; }
void lock_lock(struct spinlock *lk) {
  //   while (true) {
  //     // 如果未上锁，那么上锁，否则原地自旋
  //     if (lk->locked == 0) {
  //       lk->locked = 1;
  //       break;
  //     }
  //   }

  while (__sync_lock_test_and_set(&lk->locked, 1) != 0) {
  }
}
void lock_unlock(struct spinlock *lk) { lk->locked = 0; }