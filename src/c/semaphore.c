#include "semaphore.h"

static void InitSqQueue(SqQueue *s) {
  s->len = SEMAPHORE_MAX_VALUE;
  s->front = 0;
  s->rear = 0;
  s->size = 0;
}

static bool EnQueue(SqQueue *s, int e) {
  if (s->size == s->len) {
    return false;
  }

  s->data[s->rear] = e;
  s->rear = (s->rear + 1) % s->len;
  s->size++;
}

static bool DeQueue(SqQueue *s, int *e) {
  if (s->size == 0) {
    return false;
  }

  *e = s->data[s->front];
  s->front = (s->front + 1) % s->len;
  s->size--;
}

// 初始化信号量
void semaphore_init(semaphore *s, int value) {
  s->value = value;
  InitSqQueue(&s->queue);
}

// P操作
void semaphore_wait(semaphore *s) {
  lock_lock(&s->wait_lock);
  s->value--;
  if (s->value < 0) {
    EnQueue(&s->queue, task_get_current_id());
    task_block();
  }
  lock_unlock(&s->wait_lock);
}

void semaphore_signal(semaphore *s) {
  lock_lock(&s->signal_lock);

  s->value++;
  if (s->value <= 0) {
    int id;
    DeQueue(&s->queue, &id);
    task_wakeup(id);
  }
  lock_unlock(&s->signal_lock);
}