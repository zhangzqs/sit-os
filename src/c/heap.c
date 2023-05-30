#include "heap.h"

#include "log.h"
#include "menu.h"
#include "types.h"

// 是否从某个页面id开始是否具有连续的空闲页面
static bool _has_continuous_free_pages(struct heap *h, int pageId, int size) {
  for (int i = 0; i < size; i++) {
    if (!_is_free(h, pageId + i)) return false;
    if (_is_last(h, pageId + i)) return false;
  }
  return true;
}

static void heap_page_init(struct heap *h) {
  for (int i = 0; i < PAGES; i++) {
    _clear(h, i);
  }
}

// 实现一个页粒度的堆内存分配器
static int heap_page_malloc(struct heap *h, int size) {
  for (int i = 0; i < PAGES; i++) {
    if (_has_continuous_free_pages(h, i, size)) {
      // 找到连续的空闲页面,标记为已分配
      for (int j = 0; j < size; j++) _set_flag(h, i + j, 0b01);
      // 最后一个页面标记为末尾
      _set_flag(h, i + size - 1, 0b10);
      return i;
    }
  }
  return -1;
}

static void heap_page_free(struct heap *h, int pageId) {
  // 从pageId开始，直到遇到末尾标志
  for (int i = pageId; i < PAGES; i++) {
    if (_is_last(h, i)) {
      _clear(h, i);
      return;
    } else {
      _clear(h, i);
    }
  }
}

static void *heap_malloc(struct heap *h, size_t __size) {
  // 计算应当使用的内存页数
  int need_use_pages = __size / PAGE_SIZE;
  if (__size % PAGE_SIZE > 0) {
    need_use_pages++;
  }

  // 计算应当分配的内存页号
  int pageId = heap_page_malloc(h, need_use_pages);
  if (pageId == -1) {
    return NULL;
  }
  // 返回内存页的起始地址
  return h->data[0] + pageId * PAGE_SIZE;
}

static void heap_free(struct heap *h, void *__ptr) {
  int pageId = ((uint32_t)__ptr - (uint32_t)h->data[0]) / PAGE_SIZE;
  heap_page_free(h, pageId);
}

void heap_init() {
  globalHeap = (struct heap *)HEAP_START;
  heap_page_init(globalHeap);
}
void *malloc(size_t __size) { return heap_malloc(globalHeap, __size); }
void free(void *p) { heap_free(globalHeap, p); }

void heap_debug_list() {
  // 输出已使用的内存页分段
  int i = 0;
  while (i < PAGES) {
    if (_is_free(globalHeap, i)) {
      i++;
      continue;
    }

    int j = i;
    for (; j < PAGES; j++) {
      if (_is_last(globalHeap, j)) {
        break;
      }
    }
    printf("page %d - %d used, size: %d, addr: %p\n", i, j, j - i + 1,
           globalHeap->data[0] + i * PAGE_SIZE);
    i = j + 1;
  }
}

void heap_debug_print_meta(int line_size, int lines) {
  for (int i = 0; i < lines; i++) {
    for (int j = 0; j < line_size; j++) {
      if (_is_last(globalHeap, i * line_size + j)) {
        printf("1");
      } else {
        printf("0");
      }
      if (_is_free(globalHeap, i * line_size + j)) {
        printf("0");
      } else {
        printf("1");
      }
      printf(" ");
    }
    printf("\n");
  }
}

void heap_debug_print() {
  int line_size = 28;
  int lines = 10;
  heap_debug_print_meta(line_size, lines);
  heap_debug_list();
}

static void free_menu_item(void **udata) {
  void *p = *udata;
  free(p);
  printf("已释放内存: %p\n", p);

  struct menu *menu = *(udata + 1);
  menu->running = false;
}

static void exit_menu_item(void **udata) {
  bool *exit = *udata;
  *exit = true;
  struct menu *menu = *(udata + 1);
  menu->running = false;
}

static void render_before_action(struct menu *m) { heap_debug_print(); }

void user_heap_free() {
  struct menu m;
  bool exit = false;
  do {
    menu_init(&m, "选择待释放的内存地址: ", render_before_action);

    // 输出已使用的内存页分段
    void *udata[10][2];
    int udata_i = 0;
    int i = 0;
    while (i < PAGES) {
      if (_is_free(globalHeap, i)) {
        i++;
        continue;
      }

      int j = i;
      for (; j < PAGES; j++) {
        if (_is_last(globalHeap, j)) {
          break;
        }
      }
      void *p = globalHeap->data[0] + i * PAGE_SIZE;
      char buf[100];
      sprintf(buf, "page %d - %d used, size: %d pages, addr: %p", i, j,
              j - i + 1, p);

      udata[udata_i][0] = p;
      udata[udata_i][1] = &m;
      menu_add_item(&m, buf, free_menu_item, udata[udata_i]);
      udata_i++;

      i = j + 1;
    }
    void *udata1[2] = {&exit, &m};
    menu_add_item(&m, "退出", exit_menu_item, udata1);
    menu_loop(&m, false);
  } while (!exit);
}