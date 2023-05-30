#include "log.h"
#include "printf.h"
#include "reg.h"
#include "semaphore.h"
#include "task.h"
#include "trap.h"

#define BUFF_SIZE 5

// 缓冲池
static struct {
  // 数据数组
  int data[BUFF_SIZE];
  // 队头指针
  int front;
  // 队尾指针
  int rear;
  // 当前队列大小
  int size;
} buffer;

static void buffer_init() {
  buffer.front = 0;
  buffer.rear = 0;
  buffer.size = 0;
}

static void buffer_enqueue(int e) {
  buffer.data[buffer.rear] = e;
  buffer.rear = (buffer.rear + 1) % BUFF_SIZE;
  buffer.size++;
}

static int buffer_dequeue() {
  int e = buffer.data[buffer.front];
  buffer.front = (buffer.front + 1) % BUFF_SIZE;
  buffer.size--;
  return e;
}

static int i = 0;
static semaphore mutex_i;     // 互斥地访问变量i
static semaphore mutex_buff;  // 互斥信号量，用于互斥地访问缓冲池
static semaphore sem_consumer;  // 消费者信号量，大于0表示可以消费资源了
static semaphore sem_producer;  // 生产者信号量，大于0表示可以生产资源了
static bool running = true;

// 生产者
static void producer() {
  int pid = task_get_current_id();
  while (running) {
    semaphore_wait(&sem_producer);  // 等待缓冲区存在空闲空间，能够生产
    semaphore_wait(&mutex_buff);  // 取得缓冲池
    semaphore_wait(&mutex_i);     // 保护序号记录变量i，互斥地生产
    buffer_enqueue(i);
    log_info("producer: %d    生产产品: %d", pid, i);
    // task_delay(1000);
    i++;
    semaphore_signal(&mutex_i);       // 释放序号记录变量i
    semaphore_signal(&mutex_buff);    // 释放缓冲池
    semaphore_signal(&sem_consumer);  // 通知消费者可以消费了
  }
  task_exit();
}

// 消费者
static void consumer() {
  int pid = task_get_current_id();
  while (running) {
    semaphore_wait(&sem_consumer);  // 等待缓冲区存在空闲产品，能够消费
    semaphore_wait(&mutex_buff);  // 取得缓冲池
    log_info("consumer: %d    消费产品: %d", pid, buffer_dequeue());
    // task_delay(1000);
    semaphore_signal(&mutex_buff);    // 释放缓冲池
    semaphore_signal(&sem_producer);  // 通知生产者可以消费了
  }
  task_exit();
}

static void stopper() {
  while (true) {
    if (i < 6) {
      task_delay(10);
    } else {
      running = false;
      task_exit();
    }
  }
}

int user_producer_consumer(int argc, int argv) {
  log_info("基于信号量的生产者消费者演示启动！！！");

  running = true;
  i = 0;
  log_info("buffer size: %d", BUFF_SIZE);
  semaphore_init(&mutex_buff, 1);
  semaphore_init(&mutex_i, 1);
  semaphore_init(&sem_producer, BUFF_SIZE);  // 初始可以一次性生产BUFF_SIZE
  semaphore_init(&sem_consumer, 0);          // 初始不能消费
  task_create(producer, NULL, NULL);
  task_create(consumer, NULL, NULL);
  task_create(stopper, NULL, NULL);
  return 0;
}
