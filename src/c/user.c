#include "user.h"

#include "log.h"
#include "menu.h"
#include "printf.h"
#include "task.h"
#include "types.h"
extern void make_sure_init();

static void user_entry() {
  make_sure_init();

  clear();
  printf(
      " ________  ___  _________           ________  ________      \n|\\   "
      "____\\|\\  \\|\\___   ___\\        |\\   __  \\|\\   ____\\     \n\\ "
      "\\  "
      "\\___|\\ \\  \\|___ \\  \\_|        \\ \\  \\|\\  \\ \\  \\___|_    "
      "\n \\ \\_____  "
      "\\ \\  \\   \\ \\  \\          \\ \\  \\\\  \\ \\_____  \\   \n  "
      "\\|____|\\  \\ \\  \\  "
      " \\ \\  \\          \\ \\  \\\\  \\|____|\\  \\  \n    ____\\_\\  \\ "
      "\\__\\   "
      "\\ \\__\\          \\ \\_______\\____\\_\\  \\ \n   "
      "|\\_________\\|__|    \\|__|  "
      "         \\|_______|\\_________\\\n   \\|_________|                   "
      "    "
      "          \\|_________|\n                                             "
      " "
      "              \n                                                      "
      "      ");
  printf("\n\n正在启动OS: ");

  // for (int i = 0; i < 50; i += 1) {
  //   task_delay(200);
  //   printf("=");
  // }

  user_shell();
  task_exit();
}

void user_main() { task_create(user_entry, NULL, NULL); }