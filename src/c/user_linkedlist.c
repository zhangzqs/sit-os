#include "heap.h"
#include "log.h"
#include "menu.h"
#include "string.h"
// 编写一个程序，实现链表的创建、插入、删除、查找、遍历等功能

// 定义链表结构体
typedef struct Node {
  int data;
  struct Node *next;
} Node;

// 创建链表
static Node *createList() {
  Node *head = (Node *)malloc(sizeof(Node));
  head->next = NULL;
  return head;
}

// 插入链表
static void insertList(Node *head, int data) {
  Node *node = (Node *)malloc(sizeof(Node));
  node->data = data;
  node->next = head->next;
  head->next = node;
}

// 删除链表
static void deleteList(Node *head, int data) {
  Node *p = head;
  while (p->next != NULL) {
    if (p->next->data == data) {
      Node *temp = p->next;
      p->next = p->next->next;
      free(temp);
      return;
    }
    p = p->next;
  }
}

// 查找链表
static Node *searchList(Node *head, int data) {
  Node *p = head;
  while (p->next != NULL) {
    if (p->next->data == data) {
      return p->next;
    }
    p = p->next;
  }
  return NULL;
}

// 遍历链表
static void traverseList(Node *head) {
  Node *p = head;
  printf("遍历链表：");
  while (p->next != NULL) {
    printf("%d ", p->next->data);
    p = p->next;
  }
  printf("\n");
}

// 销毁链表
static void destroyList(Node *head) {
  Node *p = head;
  while (p->next != NULL) {
    Node *temp = p->next;
    p->next = p->next->next;
    free(temp);
  }
  free(head);
}

static Node *user_head = NULL;

static void user_option_insert() {
  while (1) {
    printf("请输入要插入的数据：");
    char *s = uart_getline();
    if (*s == 'q') {
      break;
    }
    int data = atoi(s);
    insertList(user_head, data);
  }
}

static void user_option_delete() {
  printf("请输入要删除的数据：");
  char *s = uart_getline();
  int data = atoi(s);
  deleteList(user_head, data);
}

static void user_option_search() {
  printf("请输入要查找的数据：");
  char *s = uart_getline();
  int data = atoi(s);
  Node *node = searchList(user_head, data);
  if (node != NULL) {
    printf("查找到数据：%d\n", node->data);
  } else {
    printf("未查找到数据\n");
  }
}

static void user_option_traverse() { traverseList(user_head); }

static void user_option_destroy() { destroyList(user_head); }

int user_linkedlist(int argc, char **argv) {
  log_info(
      "页粒度的基于首次最适内存分配策略实现的内存分配器实现链表演示项目启动！！"
      "！");

  user_head = createList();

  struct menu m;
  menu_init(&m, "链表演示程序", NULL);
  menu_add_item(&m, "插入数据", user_option_insert, NULL);
  menu_add_item(&m, "删除数据", user_option_delete, NULL);
  menu_add_item(&m, "查找数据", user_option_search, NULL);
  menu_add_item(&m, "遍历数据", user_option_traverse, NULL);
  menu_add_item(&m, "销毁链表", user_option_destroy, NULL);
  menu_loop(&m, true);
  return 0;
}