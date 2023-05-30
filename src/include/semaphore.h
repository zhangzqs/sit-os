#pragma once
#include <stdbool.h>

#include "lock.h"
#define SEMAPHORE_MAX_VALUE 10

typedef struct {
  // 数据数组
  int data[SEMAPHORE_MAX_VALUE];
  // 数组长度
  int len;
  // 队头指针
  int front;
  // 队尾指针
  int rear;
  // 当前队列大小
  int size;
} SqQueue;

typedef struct {
  int value;      // 当前信号量的值
  SqQueue queue;  // 阻塞队列
  struct spinlock wait_lock;
  struct spinlock signal_lock;
} semaphore;

void semaphore_init(semaphore *s, int value);
void semaphore_wait(semaphore *s);
void semaphore_signal(semaphore *s);