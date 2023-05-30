#pragma once

#include "printf.h"
#include "types.h"

extern uint32_t HEAP_START;
extern uint32_t HEAP_SIZE;
extern uint32_t TEXT_START;
extern uint32_t TEXT_END;
extern uint32_t DATA_START;
extern uint32_t DATA_END;
extern uint32_t RODATA_START;
extern uint32_t RODATA_END;
extern uint32_t BSS_START;
extern uint32_t BSS_END;

// 若想要以4K的页管理128MB的地址空间，则我们需要划分为 128 * 1024KB / (4KB/页)
// = 32*1024页
#define PAGE_SIZE 4096
#define PAGES 32 * 1024

// 堆内存数据结构
struct heap {
  uint8_t flags[PAGES];
  uint8_t data[PAGE_SIZE][PAGES];
};

// 回收该页面
static inline void _clear(struct heap *h, int pageId) { h->flags[pageId] = 0; }

// 判断是否为空闲区域
static inline bool _is_free(struct heap *h, int pageId) {
  return !(h->flags[pageId] & 0b01);
}

// 设置bit flag
static inline void _set_flag(struct heap *h, int pageId, uint8_t flags) {
  h->flags[pageId] |= flags;
}

// 是否为被分配区域的末尾
static inline bool _is_last(struct heap *h, int pageId) {
  return !!(h->flags[pageId] & 0b10);
}

void heap_init();
void *malloc(size_t __size);
void free(void *p);
void heap_debug_print();
void user_heap_free();
void heap_debug_print_meta(int line_size, int lines);
void heap_debug_list();
static struct heap *globalHeap;
