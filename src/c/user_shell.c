#include "ff.h"
#include "log.h"
#include "menu.h"
#include "printf.h"
#include "string.h"
#include "task.h"
#include "uart.h"

// 命令和命令参数的字符串最大长度
#define CMD_STR_MAX_LEN 4096

// 命令参数的最大数目
#define CMD_MAX_ARGS_LEN 10

// 最大命令数
#define CMD_MAX_SIZE 30

// 工作目录路径最大长度
#define WORK_DIR_MAX_LEN 4096
static struct command {
  char *name;
  int (*entry)(int argc, char **argv);
} _cmds[CMD_MAX_SIZE];

static int _cmd_size = 0;
static bool running = true;
static char work_dir[WORK_DIR_MAX_LEN] = "/";
static void make_path_qualify(char *result, char *subpath) {
  if (strcmp(subpath, ".") == 0) {
    strcpy(result, work_dir);
    return;
  }

  if (subpath[0] == '/') {
    strcpy(result, subpath);
    return;
  }

  strcpy(result, work_dir);
  strcat(result, subpath);

  // 确保路径以'/'结尾
  int len = strlen(result);
  if (result[len - 1] != '/') {
    result[len] = '/';
    result[len + 1] = '\0';
  }
}

void register_command(char *name, int (*entry)(int argc, char **argv)) {
  log_debug("register command %s", name);
  struct command *p = &_cmds[_cmd_size++];
  p->name = name;
  p->entry = entry;
}

struct command *_find_cmd(char *name) {
  for (int i = 0; i < _cmd_size; i++) {
    if (strcmp(_cmds[i].name, name) == 0) {
      return &_cmds[i];
    }
  }
  return NULL;
}

static const char *fresult_enum_to_string(int fr) {
  const char *arr[] = {
      "Succeeded",
      "A hard error occurred in the low level disk I/O layer",
      "Assertion failed",
      "The physical drive cannot work",
      "Could not find the file",
      "Could not find the path",
      "The path name format is invalid",
      "Access denied due to prohibited access or directory full",
      "Access denied due to prohibited access",
      "The file/directory object is invalid",
      "The physical drive is write protected",
      "The logical drive number is invalid",
      "The volume has no work area",
      "There is no valid FAT volume",
      "The f_mkfs() aborted due to any parameter error",
      "Could not get a grant to access the volume within defined period",
      "The operation is rejected according to the file sharing policy",
      "LFN working buffer could not be allocated",
      "Number of open files > _FS_SHARE",
      "Given parameter is invalid"};
  if (fr < 0 || fr > 20) {
    return "Unknown error";
  }
  return arr[fr];
}

int exec(char *cmdline) {
  // 如果为空字符串或空格字符串，直接返回
  if (strlen(cmdline) == 0 || strcmp(cmdline, " ") == 0) {
    return 0;
  }
  char *s_mem;

  // 解析命令
  char *cmd_name = strtok_with_ch_mem(&s_mem, cmdline, " ");
  struct command *p = _find_cmd(cmd_name);
  if (p == NULL) {
    printf("command %s not found\n", cmd_name);
    return -1;
  }
  // 解析参数
  int argc = 1;
  char *argv[CMD_MAX_ARGS_LEN];
  argv[0] = cmd_name;
  while (argc < CMD_MAX_ARGS_LEN) {
    char *arg = strtok_with_ch_mem(&s_mem, NULL, " ");
    if (arg == NULL) {
      break;
    }
    argv[argc++] = arg;
  }

  log_trace("argc: %d", argc);
  for (int i = 0; i < argc; i++) {
    log_trace("argv[%d]: %s", i, argv[i]);
  }

  // 执行命令
  return p->entry(argc, argv);
  ;
}

void run_shell() {
  while (running) {
    // 显示工作目录
    printf("\x1b[90m%s>\x1b[0m ", work_dir);

    // 获取命令行 read
    char *line = uart_getline();

    // 执行命令 execute
    exec(line);
  }
}

int exec_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: exec [-o <file.txt>] <cmd> <arguments...>\n");
    return -1;
  }
  // 是否输出到文件
  bool output_to_file = strcmp(argv[1], "-o") == 0;
  char *output_file = NULL;
  char buf[10240];
  memset(buf, 0, sizeof(buf));
  char *touch_argv[2] = {"touch", NULL};

  if (output_to_file) {
    if (argc < 4) {
      printf("Usage: exec [-o <file.txt>] <cmd> <arguments...>\n");
      return -1;
    }
    output_file = argv[2];
    touch_argv[1] = output_file;
    argv += 3;
    argc -= 3;
    redirect_printf(buf);
  } else {
    argv += 1;
    argc -= 1;
  }

  // 解析命令
  char *cmd_name = argv[0];
  struct command *p = _find_cmd(cmd_name);
  if (p == NULL) {
    printf("command %s not found\n", cmd_name);
    restore_printf();

    return -1;
  }
  // 执行命令
  int return_code = p->entry(argc, argv);

  if (output_to_file) {
    // 创建文件
    touch_cmd(2, touch_argv);
    // 写入文件
    FIL file;
    FRESULT res = f_open(&file, output_file, FA_WRITE);
    if (res) {
      log_error("[ERROR] f_open failed: %s[%s]", fresult_enum_to_string(res),
                output_file);
      restore_printf();

      return -1;
    }
    UINT written;
    f_lseek(&file, f_size(&file));
    res = f_write(&file, buf, strlen(buf), &written);
    if (res) {
      log_error("[ERROR] f_write failed: %s[%s]", fresult_enum_to_string(res),
                output_file);
      restore_printf();
      return -1;
    }
    f_close(&file);
    restore_printf();
  }

  return return_code;
}

int watch_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: watch <cmd> <arguments...>\n");
    return -1;
  }
  while (true) {
    clear();
    printf("Press q to quit!!!\n");
    exec_cmd(argc, argv);
    char c = uart_getch_nowait();
    if (c == 'q') {
      break;
    }
    task_delay(1000);
  }
  return 0;
}
int echo_cmd(int argc, char **argv) {
  for (int i = 1; i < argc; i++) {
    printf("%s ", argv[i]);
  }
  printf("\n");
  return 0;
}

int logset_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: logset <level>\n");
    return -1;
  }

  char *level = argv[1];
  // 将level字符串中的字母全部转化为大写
  for (int i = 0; i < strlen(level); i++) {
    level[i] = toupper(level[i]);
  }

  for (enum level l = LOG_TRACE; l <= LOG_FATAL; l++) {
    if (strcmp(level, log_level_string(l)) == 0) {
      log_set_level(l);
      printf("log set successful: %s\n", level);
      return 0;
    }
  }
  printf("invalid log level: %s", level);
  return -1;
}

int log_cmd(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: log <level> <message>\n");
    return -1;
  }

  char *level = argv[1];
  // 将level字符串中的字母全部转化为大写
  for (int i = 0; i < strlen(level); i++) {
    level[i] = toupper(level[i]);
  }
  char *msg = argv[2];
  for (enum level l = LOG_TRACE; l <= LOG_FATAL; l++) {
    if (strcmp(level, log_level_string(l)) == 0) {
      log(l, "%s", msg);
      printf("\n");
      return 0;
    }
  }
  printf("invalid log level: %s", level);
}

int help_cmd(int argc, char **argv) {
  // 列出所有可用命令
  printf("Available commands[%d]:\n", _cmd_size);
  for (int i = 0; i < _cmd_size; i++) {
    printf("  %s\n", _cmds[i].name);
  }
  return 0;
}

int ls_cmd(int argc, char **argv) {
  char *path = work_dir;
  if (argc == 2) {
    path = argv[1];
  }
  DIR dir;
  FRESULT res = f_opendir(&dir, path);
  if (res) {
    log_error("[ERROR] f_opendir failed: %s", fresult_enum_to_string(res));
    return -1;
  }

  FILINFO fno;
  res = f_readdir(&dir, &fno);
  if (res) {
    log_error("[ERROR] f_readdir failed: %s", fresult_enum_to_string(res));
    return -1;
  }
  FRESULT fr = f_findfirst(&dir, &fno, path, "*");

  while (fr == FR_OK && fno.fname[0]) {
    if (fno.fattrib == AM_DIR) {
      printf("\x1b[36m%s\x1b[0m\n", fno.fname);
    } else {
      if (endswith(fno.fname, ".bat")) {
        printf("\x1b[32m%s\x1b[0m\n", fno.fname);
      } else {
        printf("%s\n", fno.fname);
      }
    }
    fr = f_findnext(&dir, &fno);
  }
  return 0;
}

int open_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: open <file>\n");
    return -1;
  }

  char path[WORK_DIR_MAX_LEN];
  strcpy(path, work_dir);
  strcat(path, argv[1]);

  FIL file;
  FRESULT res = f_open(&file, path, FA_READ);
  if (res) {
    log_error("[ERROR] f_open failed: %s", fresult_enum_to_string(res));
    return -1;
  }
  char buf[1024];
  UINT read;
  res = f_read(&file, buf, 1024, &read);
  if (res) {
    log_error("[ERROR] f_read failed: %s", fresult_enum_to_string(res));
    return -1;
  }
  buf[read] = '\0';
  printf("%s\n", buf);
  f_close(&file);
  return 0;
}

int touch_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: touch <file> ...\n");
    return -1;
  }

  for (int i = 1; i < argc; i++) {
    char path[WORK_DIR_MAX_LEN];
    make_path_qualify(path, argv[i]);

    FIL file;
    FRESULT res = f_open(&file, path, FA_CREATE_ALWAYS);
    if (res) {
      log_error("[ERROR] f_open failed: %s", fresult_enum_to_string(res));
      return -1;
    }
    f_close(&file);
  }
}

int append_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: append <file> <content>\n");
    return -1;
  }

  FIL file;
  char path[WORK_DIR_MAX_LEN];
  make_path_qualify(path, argv[1]);
  FRESULT res = f_open(&file, path, FA_WRITE);
  if (res) {
    log_error("[ERROR] f_open failed: %s", fresult_enum_to_string(res));
    return -1;
  }
  UINT written;
  for (int i = 2; i < argc; i++) {
    f_lseek(&file, f_size(&file));
    res = f_write(&file, argv[i], strlen(argv[i]), &written);
    if (res) {
      log_error("[ERROR] f_write failed: %s", fresult_enum_to_string(res));
      return -1;
    }
    if (i != argc - 1) {
      res = f_write(&file, " ", 1, &written);
      if (res) {
        log_error("[ERROR] f_write failed: %s", fresult_enum_to_string(res));
        return -1;
      }
    }
  }
  f_close(&file);
  return 0;
}

int appendln_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: appendln <file> <content>\n");
    return -1;
  }
  if (append_cmd(argc, argv)) {
    return -1;
  }

  char *argv1[] = {"appendln", argv[1], "\n"};
  if (append_cmd(3, argv1)) {
    return -1;
  }
  return 0;
}

int bat_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: bat <file>\n");
    return -1;
  }

  char path[WORK_DIR_MAX_LEN];
  make_path_qualify(path, argv[1]);

  FIL file;
  FRESULT res = f_open(&file, path, FA_READ);
  if (res) {
    log_error("[ERROR] f_open failed: %s[%s]", fresult_enum_to_string(res),
              path);
    return -1;
  }
  char buf[1024];
  UINT read;
  res = f_read(&file, buf, 1024, &read);
  if (res) {
    log_error("[ERROR] f_read failed: %s[%s]", fresult_enum_to_string(res),
              path);
    return -1;
  }
  // 依次遍历buf中的以\n分割的所有命令，并调用exec执行, 不使用strtok
  char *s_mem;
  char *p = strtok_with_ch_mem(&s_mem, buf, "\n");

  do {
    if (strlen(p) != 0) {
      printf("\x1b[90m%s>\x1b[0m %s\n", work_dir, p);
    }
    char cmdline[1024];
    strcpy(cmdline, p);
    exec(cmdline);
    p = strtok_with_ch_mem(&s_mem, NULL, "\n");
  } while (p != NULL && p - buf < read);

  f_close(&file);
  return 0;
}

int cd_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: cd <path>\n");
    return -1;
  }
  char *path = argv[1];

  // 当前目录
  if (strcmp(path, ".") == 0) {
    return 0;
  }

  // 上一级目录
  if (strcmp(path, "..") == 0) {
    if (strcmp(work_dir, "/") == 0) {
      return 0;
    }

    // 删除最后两个/
    for (int i = 0; i < 2; i++) {
      char *p = strrchr(work_dir, '/');
      if (p == NULL) return 0;
      *(p + i) = '\0';
    }

    return 0;
  }

  // 子目录
  char work_dir_copy[WORK_DIR_MAX_LEN];
  make_path_qualify(work_dir_copy, path);

  FILINFO fno;
  FRESULT fr = f_stat(work_dir_copy, &fno);
  if (fr == FR_NO_FILE) {
    printf("%s is not exist.\n", work_dir_copy);
    return -1;
  }
  if (fr == FR_OK) {
    if (fno.fattrib != AM_DIR) {
      printf("%s is not a directory.\n", work_dir_copy);
      return -1;
    }

    // 检查是否有末尾的斜杠
    if (work_dir_copy[strlen(work_dir_copy) - 1] != '/') {
      strcat(work_dir_copy, "/");
    }
    strcpy(work_dir, work_dir_copy);
    return 0;
  }

  printf("unknown error. (%d)\n", fr);
}

int mkdir_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: mkdir <path>\n");
    return -1;
  }
  char path[WORK_DIR_MAX_LEN];
  make_path_qualify(path, argv[1]);

  FRESULT res = f_mkdir(path);

  if (res) {
    printf("[ERROR] f_mkdir failed: %s\n", fresult_enum_to_string(res));
    return -1;
  }
  return 0;
}

int rm_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: rm <path>\n");
    return -1;
  }
  char path[WORK_DIR_MAX_LEN];
  make_path_qualify(path, argv[1]);

  FRESULT res = f_unlink(path);
  if (res) {
    printf("[ERROR] rm failed: %s\n", fresult_enum_to_string(res));
    return -1;
  }
  return 0;
}

static void assert(bool cond) {
  if (!cond) {
    panic("assertion failed");
  }
}

FRESULT rmdir(const TCHAR *path) {
  FRESULT res;
  DIR dir;
  FILINFO fno;
  TCHAR *fn;
  TCHAR pathbuf[255];

  res = f_opendir(&dir, path);  // 打开文件夹
  if (res) {
    return res;
  }
  while (f_readdir(&dir, &fno) == FR_OK && fno.fname[0]) {
    fn = fno.fname;
    sprintf(pathbuf, "%s/%s", path, fn);  // 构建文件或文件夹的路径
    if (fno.fattrib & AM_DIR) {  // 如果是文件夹，则递归删除
      res = rmdir(pathbuf);
      if (res) {
        return res;
      }
    } else {  // 如果是文件，则删除
      res = f_unlink(pathbuf);
      if (res) {
        return res;
      }
    }
  }
  f_closedir(&dir);  // 关闭文件夹
  if (res == FR_OK) {
    res = f_unlink(path);  // 删除文件夹本身
  }
  return res;
}

int rmdir_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: rmdir <path>\n");
    return -1;
  }
  char path[WORK_DIR_MAX_LEN];
  make_path_qualify(path, argv[1]);
  rmdir(path);
  return 0;
}

// 递归地列举目录树
static void list_dir_tree(TCHAR *path, int depth) {
  DIR dir;
  FILINFO fno;
  TCHAR subpath[256];
  FRESULT res;

  res = f_findfirst(&dir, &fno, path, "*");
  assert(res == FR_OK);

  while (res == FR_OK && fno.fname[0]) {
    if (fno.fattrib & AM_DIR) {
      for (int i = 0; i < depth; i++) printf("  ");
      printf("\x1b[36m%s/\x1b[0m\n", fno.fname);

      sprintf(subpath, "%s/%s", path, fno.fname);
      list_dir_tree(subpath, depth + 1);
    } else {
      for (int i = 0; i < depth; i++) printf("  ");
      if (endswith(fno.fname, ".bat")) {
        printf("\x1b[32m%s\x1b[0m\n", fno.fname);
      } else {
        printf("%s\n", fno.fname);
      }
    }
    res = f_findnext(&dir, &fno);
    assert(res == FR_OK);
  }
}

int tree_cmd(int argc, char **argv) {
  list_dir_tree(work_dir, 0);
  return 0;
}

int rand_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: rand <max>\n");
    return -1;
  }
  int max = atoi(argv[1]);
  printf("%d\n", rand() % max);
  return 0;
}

int clear_cmd(int argc, char **argv) {
  clear();
  return 0;
}

extern int user_producer_consumer(int argc, char **argv);
extern int user_linkedlist(int argc, char **argv);
extern int user_filesystem(int argc, char **argv);
extern int user_heap(int argc, char **argv);
extern int user_calc(int argc, char **argv);
extern int user_maze(int argc, char **argv);

static int hanoi_i = 0;

static void hanoi(int n, char a, char b, char c) {
  if (n == 1) {
    printf("%d: %c -> %c\n", hanoi_i++, a, c);
  } else {
    hanoi(n - 1, a, c, b);
    printf("%d: %c -> %c\n", hanoi_i++, a, c);
    hanoi(n - 1, b, a, c);
  }
}

int user_hanoi(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: hanoi <n>\n");
    return -1;
  }
  int n = atoi(argv[1]);
  hanoi_i = 1;
  hanoi(n, 'A', 'B', 'C');
  return 0;
}

static void user_heap_menu_item(void *arg) {
  int argc = 2;
  char *argv[] = {"heap", "ui"};
  user_heap(argc, argv);
}

void user_filesystem_menu_item(void *arg) {
  int argc = 1;
  char *argv[] = {"filesystem"};
  user_filesystem(argc, argv);
}

int user_demo(int argc, char *argv) {
  struct menu m;
  menu_init(&m, "Please choose a user program demo:", NULL);
  menu_add_item(&m, "Linked List", user_linkedlist, NULL);
  menu_add_item(&m, "Heap Memory", user_heap_menu_item, NULL);
  menu_add_item(&m, "File System", user_filesystem_menu_item, NULL);
  menu_add_item(&m, "Producer-Consumer", user_producer_consumer, NULL);
  menu_loop(&m, true);
  return 0;
}

void user_init_script() {
  const char *cmds[] = {
      "touch sh.bat",
      "appendln sh.bat echo hello world",
      "appendln sh.bat mkdir abc",
      "appendln sh.bat mkdir abc/cde",
      "appendln sh.bat touch file.txt",
      "appendln sh.bat appendln file.txt hello world",
      "appendln sh.bat cat file.txt",
      "appendln sh.bat ls",
      "appendln sh.bat cd abc",
      "appendln sh.bat touch script.bat",
      "appendln sh.bat appendln script.bat echo hello world1",
      "appendln sh.bat appendln script.bat echo hello world2",
      "appendln sh.bat bat script.bat",
      "appendln sh.bat cd ..",
      "appendln sh.bat tree",
      "appendln sh.bat heap malloc 4097",
      "appendln sh.bat heap malloc 9999",
      "appendln sh.bat heap malloc 8",
      "bat sh.bat",
      "bat abc/script.bat",
  };
  for (int i = 0; i < sizeof(cmds) / sizeof(cmds[0]); i++) {
    exec(cmds[i]);
  }
}

static void counter_process() {
  uint32_t *c = task_get_current_params();
  while (true) {
    *c++;
  }
  task_exit();
}
static uint32_t counter[10];
static int counter_i = 0;

// 开辟后台进程不断地进行后台计数
int counter_cmd(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: counter <create|list>\n");
    return -1;
  }
  if (strcmp("create", argv[1]) == 0) {
    if (counter_i >= 10) {
      printf("Too many counters!\n");
      return -1;
    }

    counter[counter_i] = 0;
    task_create(counter_process, &counter[counter_i], "counter");
    counter_i++;
    return 0;
  }
  if (strcmp("list", argv[1]) == 0) {
    for (int i = 0; i < counter_i; i++) {
      printf("counter %d: %d\n", i, counter[i]);
    }
    return 0;
  }

  return 0;
}

void user_shell() {
  _cmd_size = 0;
  running = true;
  register_command("echo", echo_cmd);
  register_command("help", help_cmd);
  register_command("ls", ls_cmd);
  register_command("open", open_cmd);
  register_command("cat", open_cmd);
  register_command("cd", cd_cmd);
  register_command("touch", touch_cmd);
  register_command("mkdir", mkdir_cmd);
  register_command("log", log_cmd);
  register_command("logset", logset_cmd);
  register_command("tree", tree_cmd);
  register_command("append", append_cmd);
  register_command("appendln", appendln_cmd);
  register_command("exec", exec_cmd);
  register_command("bat", bat_cmd);
  register_command("rm", rm_cmd);
  register_command("rmdir", rmdir_cmd);
  register_command("clear", clear_cmd);
  register_command("rand", rand_cmd);
  register_command("demo", user_demo);
  register_command("linkedlist", user_filesystem);
  register_command("pc", user_producer_consumer);
  register_command("filesystem", user_filesystem);
  register_command("heap", user_heap);
  register_command("calc", user_calc);
  register_command("hanoi", user_hanoi);
  register_command("maze", user_maze);
  register_command("taskmgr", taskmgr_cmd);
  register_command("counter", counter_cmd);
  register_command("watch", watch_cmd);
  user_init_script();
  run_shell();
}
