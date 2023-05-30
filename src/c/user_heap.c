#include "heap.h"
#include "menu.h"
void user_heap_malloc() {
  while (1) {
    printf("请输入要分配的内存大小: ");
    char *s = uart_getline();
    if (strcmp(s, "q") == 0) {
      return;
    }
    int size = atoi(s);
    void *p = malloc(size);
    printf("分配的内存地址为: %p\n", p);
  }
}

static void ui() {
  struct menu m;
  menu_init(&m, "内存管理演示程序", NULL);

  menu_add_item(&m, "内存分配", user_heap_malloc, NULL);
  menu_add_item(&m, "内存释放", user_heap_free, NULL);

  menu_loop(&m, true);
}

int user_heap(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s malloc|free|ui [size|addr]\n", argv[0]);
    return -1;
  }
  if (strcmp(argv[1], "malloc") == 0) {
    void *p = malloc(atoi(argv[2]));
    printf("分配的内存地址为: %p\n", p);
    return 0;
  }
  if (strcmp(argv[1], "free") == 0) {
    void *p = hex2uint32(argv[2]);
    free(p);
    printf("已释放内存: %p\n", p);
    return 0;
  }
  if (strcmp(argv[1], "list") == 0) {
    heap_debug_list();
    return 0;
  }
  if (strcmp(argv[1], "debug") == 0) {
    heap_debug_print();
    return 0;
  }
  if (strcmp(argv[1], "ui") == 0) {
    ui();
    return 0;
  }
  printf("Usage: %s malloc|free|ui [size|addr]\n", argv[0]);
  return -1;
}