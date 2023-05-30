#include "task.h"

#include "context.h"
#include "lock.h"
#include "log.h"
#include "platform.h"
#include "printf.h"
#include "reg.h"
#include "string.h"
#include "types.h"

#define STACK_SIZE 512 * 1024
#define MAX_TASKS 10

// 当前任务数
static int _size = 0;

// 当前任务指针
static int _current = -1;

enum task_state { READY, BLOCKED, RUNNING, END };

char *task_state_to_string(enum task_state s) {
  switch (s) {
    case READY:
      return "READY";
    case BLOCKED:
      return "BLOCKED";
    case RUNNING:
      return "RUNNING";
    case END:
      return "END";
    default:
      return "UNKNOWN_STATE";
  }
}

static struct task {
  // 任务id
  uint8_t id;
  // 任务参数
  void *params;
  // 任务栈
  uint8_t stack[STACK_SIZE];
  // 任务CPU上下文
  struct context context;
  // 当前任务状态
  enum task_state state;
  // 任务描述，用于任务管理器中显示
  char *desc;
  // 任务获得的CPU时间片
  uint32_t ticks;
} _tasks[MAX_TASKS];

static struct spinlock _lock;

void schedule() {
  // log_debug("schedule: size(%d) current(%d)", _size, _current);
  if (_size <= 0) {
    log_fatal("No task should be schedule");
    return;
  }

  // 关闭中断, 保证在任务调度器的原子性
  lock_lock(&_lock);

  // 第一次被调度时候_current == -1，若_size不为0，则可调度至第0任务
  if (_current == -1) {
    _current = 0;
  }

  // 使当前任务从运行态回到就绪态
  if (_tasks[_current].state == RUNNING) {
    _tasks[_current].state = READY;
  }

  // 直到找到一个就绪态的任务
  while (true) {
    _current = (_current + 1) % MAX_TASKS;
    if (_tasks[_current].state == READY) {
      break;
    }
  }
  // 切换到该任务运行
  _tasks[_current].state = RUNNING;
  _tasks[_current].ticks++;
  // 开中断
  lock_unlock(&_lock);

  switch_to(&_tasks[_current].context);
}
// 使当前任务进入阻塞状态
void task_block() {
  log_debug("task id %d is blocked", _current);

  lock_lock(&_lock);
  _tasks[_current].state = BLOCKED;
  lock_unlock(&_lock);

  task_yield();
}

// 唤醒某个任务
void task_wakeup(int id) {
  log_debug("task id %d is ready", id);
  lock_lock(&_lock);
  _tasks[id].state = READY;
  lock_unlock(&_lock);
}

// 初始化任务调度器
void task_init() {
  reg_w_mscratch(0);
  reg_w_mie(reg_r_mie() | MIE_MSIE);

  lock_init(&_lock);
  // 初始化所有任务状态为END
  for (int i = 0; i < MAX_TASKS; i++) {
    _tasks[i].state = END;
  }
}

int task_create(TaskEntry entry, void *params, void *desc) {
  // 任务超过最大数
  if (_size >= MAX_TASKS) {
    return -1;
  }

  lock_lock(&_lock);
  int i;
  // 寻找到第一个状态为END的任务
  for (i = 0; i < MAX_TASKS; i++) {
    if (_tasks[i].state == END) {
      break;
    }
  }

  log_debug("task id %d is created", i);
  memset(_tasks + i, 0, sizeof(struct task));
  _tasks[i].id = i;
  _tasks[i].params = params;
  _tasks[i].state = READY;
  _tasks[i].context.sp = (reg_t)&_tasks[_size].stack[STACK_SIZE - 1];
  _tasks[i].context.pc = (reg_t)entry;
  _tasks[i].ticks = 0;
  _tasks[i].desc = "unknown";
  _size++;
  lock_unlock(&_lock);

  return 0;
}

// 获取当前任务id
int task_get_current_id() { return _current; }

// 获取当前任务参数
void *task_get_current_params() { return _tasks[_current].params; }

void task_yield() {
  // 触发一个机器模式下的软件中断
  int id = reg_r_mhartid();
  *(uint32_t *)CLINT_MSIP(id) = 1;
}

void task_delay(int count) {
  count *= 50000;
  while (count--) {
  }
}

void task_exit() {
  log_debug("task id %d is end", _current);
  lock_lock(&_lock);
  _tasks[_current].state = END;
  _size--;
  lock_unlock(&_lock);
  task_yield();
}

static void list_all_task() {
  printf("id\tdesc\tticks\tstate\n");
  for (int i = 0; i < MAX_TASKS; i++) {
    if (_tasks[i].state == END) {
      continue;
    }
    printf("%d\t%s\t%d\t%s\n", _tasks[i].id, _tasks[i].desc, _tasks[i].ticks,
           task_state_to_string(_tasks[i].state));
  }
}
/*
 * 一个任务管理器用户程序，可以查看所有后台进程的状态，可以杀死进程
 */
int taskmgr_cmd(int argc, char **argv) {
  if (argc == 1) {
    list_all_task();
    return 0;
  }
  char *cmd = argv[1];
  if (strcmp(cmd, "kill") == 0) {
    if (argc != 3) {
      printf("Usage: taskmgr kill <id>\n");
      return 0;
    }
    int id = atoi(argv[2]);
    if (id < 0 || id >= MAX_TASKS) {
      printf("Invalid task id\n");
      return 0;
    }
    if (_tasks[id].state == END) {
      printf("Task %d is not exist\n", id);
      return 0;
    }
    if (id == _current) {
      printf("Can't kill current task\n");
      return 0;
    }
    _tasks[id].state = END;
    _size--;
    return 0;
  }
  return 0;
}
