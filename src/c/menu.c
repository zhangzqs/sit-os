#include "menu.h"

#include "printf.h"
#include "types.h"
#include "uart.h"

void menu_init(struct menu *menu, char *title, render_before before) {
  strcpy(menu->title, title);
  menu->size = 0;
  menu->choice = 0;
  menu->running = true;
  menu->before = before;
}

static void menu_exit_item(struct menu *menu) { menu->running = false; }

void menu_add_item(struct menu *menu, char *item, menu_action action,
                   void *userdata) {
  strcpy(menu->items[menu->size], item);
  menu->actions[menu->size] = action;
  menu->userdatas[menu->size] = userdata;
  menu->size++;
}

void menu_show(struct menu *menu) {
  clear();
  if (menu->before != NULL) {
    menu->before(menu);
  }

  printf("%s\n", menu->title);
  for (int i = 0; i < menu->size; i++) {
    if (i == menu->choice) {
      printf("\x1b[36m%d. %s\x1b[0m\n", i, menu->items[i]);
    } else {
      printf("%d. %s\n", i, menu->items[i]);
    }
  }
  printf("Use 'w', 's' to move choice\n");
  printf("Use 'q' to exit\n");
  printf("Use 'Enter' to run choice\n");
}

void menu_loop(struct menu *menu, bool addExitItem) {
  if (addExitItem) {
    menu_add_item(menu, "Exit", menu_exit_item, menu);
  }
  do {
    menu_show(menu);
    switch (uart_getch()) {
      case 'q':
        return;
      case 'w':
        menu->choice = (menu->choice - 1 + menu->size) % menu->size;
        break;
      case 's':
        menu->choice = (menu->choice + 1) % menu->size;
        break;
      case '\r': {
        menu->actions[menu->choice](menu->userdatas[menu->choice]);
        if (menu->running) {
          printf("Press any key to continue\n");
          uart_getch();
        }
        break;
      }
      default:
        break;
    }
  } while (menu->running);
}