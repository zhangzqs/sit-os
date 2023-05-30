#pragma once

struct spinlock {
  int locked;
};

void lock_init(struct spinlock *);
void lock_lock(struct spinlock *);
void lock_unlock(struct spinlock *);
