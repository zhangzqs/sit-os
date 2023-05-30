#include "string.h"

#define MAX_MENU_ITEMS 10
#define MAX_STRING_LENGTH 100

typedef void (*menu_action)(void *userdata);
typedef void (*render_before)(struct menu *menu);

struct menu {
  char title[MAX_STRING_LENGTH];
  char items[MAX_MENU_ITEMS][MAX_STRING_LENGTH];
  menu_action actions[MAX_MENU_ITEMS];
  render_before before;
  void *userdatas[MAX_MENU_ITEMS];
  int size;
  int choice;
  bool running;
};

void menu_init(struct menu *menu, char *title, render_before before);

void menu_add_item(struct menu *menu, char *item, menu_action action,
                   void *userdata);

void menu_show(struct menu *menu);

void menu_loop(struct menu *menu, bool addExitItem);
