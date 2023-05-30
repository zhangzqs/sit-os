#include "diskio.h"
#include "ff.h"
#include "log.h"
#include "menu.h"
#include "string.h"
#include "uart.h"

static void exit(int code) { task_exit(); }
static void assert(bool cond) {
  if (!cond) {
    panic("assertion failed");
  }
}

static void create_file() {
  // 创建文件
  FIL fp;
  FRESULT res;

  res = f_open(&fp, "test.txt", FA_CREATE_ALWAYS | FA_WRITE);
  if (res) {
    log_error("[ERROR] f_open failed: %d", res);
    exit(1);
  }
  log_info("create file success!");

  // 写入文件
  const char *str = "Hello, world!";
  const int str_len = 14;
  UINT bytes_written;
  f_write(&fp, str, str_len, &bytes_written);
  if (bytes_written != str_len) {
    log_error("[ERROR] f_write failed: %d", res);
    exit(1);
  }
  log_info("write file success!");

  // 关闭文件
  res = f_close(&fp);
  if (res) {
    log_error("[ERROR] f_close failed: %d", res);
    exit(1);
  }
  log_info("close file success!");

  // 判断文件是否存在
  FILINFO fno;
  res = f_stat("test.txt", &fno);
  if (res) {
    log_error("[ERROR] f_stat failed: %d", res);
    exit(1);
  }
  log_info("f_stat test success!");
  // 输出文件信息
  log_info("File size: %d", fno.fsize);
  log_info(
      "Attributes: %c%c%c%c%c", (fno.fattrib & AM_DIR) ? 'D' : '-',
      (fno.fattrib & AM_RDO) ? 'R' : '-', (fno.fattrib & AM_HID) ? 'H' : '-',
      (fno.fattrib & AM_SYS) ? 'S' : '-', (fno.fattrib & AM_ARC) ? 'A' : '-');
  log_info("File name: %s", fno.fname);

  FSIZE_t sz = f_size(&fp);
  if (sz != str_len) {
    log_error("[ERROR] f_size failed: %d", res);
    exit(1);
  }
  log_info("f_size test success!");
}

static void open_file() {
  FIL fp;
  FRESULT res;

  res = f_open(&fp, "test.txt", FA_READ);
  if (res) {
    log_error("[ERROR] f_open failed: %d", res);
    exit(1);
  }
  log_info("open file success!");
  // 读取文件大小
  FSIZE_t sz = f_size(&fp);

  // 创建缓冲区
  char buff[sz];

  // 读取文件
  UINT bytes_read;
  res = f_read(&fp, buff, sizeof(buff), &bytes_read);
  if (res) {
    log_error("[ERROR] f_read failed: %d", res);
    exit(1);
  }
  log_info("read file success!");
  log_info("bytes_read: %d", bytes_read);

  // 输出文件内容
  log_info("File content: %s", buff);

  res = f_close(&fp);
  if (res) {
    log_error("[ERROR] f_close failed: %d", res);
    exit(1);
  }
  log_info("close file success!");
}

// 创建目录
static void create_dir() {
  FRESULT res = f_mkdir("test_dir");
  if (res) {
    log_error("[ERROR] f_mkdir failed: %d", res);
    exit(1);
  }
  log_info("create dir success!");
}

// 目录列举
static void list_dir(char *s) {
  DIR dir;
  FRESULT res = f_opendir(&dir, s);
  if (res) {
    log_error("[ERROR] f_opendir failed: %d", res);
  }
  log_info("open dir success!");

  FILINFO fno;
  res = f_readdir(&dir, &fno);
  if (res) {
    log_error("[ERROR] f_readdir failed: %d", res);
  }
  log_info("read dir success!");
  FRESULT fr =
      f_findfirst(&dir, &fno, "", "*"); /* Start to search for photo files */

  while (fr == FR_OK && fno.fname[0]) { /* Repeat while an item is found */
    log_info("find[%s]: %s", fno.fattrib == AM_DIR ? "DIR" : "FILE",
             fno.fname);         /* Print the object name */
    fr = f_findnext(&dir, &fno); /* Search for next item */
  }
}

/*
创建一个目录树，计划创建如下目录结构
dir1/
  dir11/
    file111
    file112
  dir12/
    file121
    file122
dir2/
  dir21/
    file211
    file212
  dir22/
    file221
    file222
*/
static void create_dir_tree() {
  FIL fp;
  f_mkdir("dir1");
  {
    f_mkdir("dir1/dir11");
    {
      f_open(&fp, "dir1/dir11/file111", FA_CREATE_ALWAYS | FA_WRITE);
      f_open(&fp, "dir1/dir11/file112", FA_CREATE_ALWAYS | FA_WRITE);
    }
    f_mkdir("dir1/dir12");
    {
      f_open(&fp, "dir1/dir12/file121", FA_CREATE_ALWAYS | FA_WRITE);
      f_open(&fp, "dir1/dir12/file122", FA_CREATE_ALWAYS | FA_WRITE);
    }
  }
  f_mkdir("dir2");
  {
    f_mkdir("dir2/dir21");
    {
      f_open(&fp, "dir2/dir21/file211", FA_CREATE_ALWAYS | FA_WRITE);
      f_open(&fp, "dir2/dir21/file212", FA_CREATE_ALWAYS | FA_WRITE);
    }
    f_mkdir("dir2/dir22");
    {
      f_open(&fp, "dir2/dir22/file221", FA_CREATE_ALWAYS | FA_WRITE);
      f_open(&fp, "dir2/dir22/file222", FA_CREATE_ALWAYS | FA_WRITE);
    }
  }
  log_info("create dir tree success!");
}

// 递归地列举目录树
static void list_dir_tree(TCHAR *path, int depth) {
  DIR dir;
  FILINFO fno;
  TCHAR subpath[256];
  FRESULT res;

  res = f_findfirst(&dir, &fno, path, "*");
  assert(res == RES_OK);

  while (res == FR_OK && fno.fname[0]) {
    if (fno.fattrib == AM_DIR) {
      for (int i = 0; i < depth; i++) printf("  ");
      printf("%s/\n", fno.fname);

      sprintf(subpath, "%s/%s", path, fno.fname);
      list_dir_tree(subpath, depth + 1);
    } else {
      for (int i = 0; i < depth; i++) printf("  ");
      printf("%s\n", fno.fname);
    }
    res = f_findnext(&dir, &fno);
    assert(res == FR_OK);
  }
}

static int op1() {
  // 挂载磁盘
  FATFS fs;
  FRESULT res = f_mount(&fs, "0:", 0);
  if (res) {
    log_error("f_mount failed: %d", res);
    exit(1);
  }

  // 创建文件
  create_file();

  // 打开文件
  open_file();

  // 创建目录
  create_dir();

  // 列举目录
  list_dir("");
  return 0;
}

static int op2() {
  create_dir_tree();     // 创建目录树
  list_dir_tree("", 0);  // 递归地列举目录树
  return 0;
}

void user_filesystem1() {
  log_info("文件系统操作演示项目启动！！！");

  log_info("文件操作：");
  op1();

  log_info("目录递归遍历操作：");
  op2();
}

void user_option_create_file() {
  printf("请输入文件名：\n");
  char filename[50];
  char *s = uart_getline();
  strcpy(filename, s);

  FIL fp;
  FRESULT res = f_open(&fp, s, FA_CREATE_ALWAYS | FA_WRITE);
  if (res) {
    log_error("f_open failed: %d", res);
    exit(1);
  }
  log_info("create file success!");

  printf("请输入文件内容：");
  s = uart_getline();
  // 写入文件
  const char *str = s;
  const int str_len = strlen(str) + 1;
  UINT bytes_written;
  f_write(&fp, str, str_len, &bytes_written);
  if (bytes_written != str_len) {
    log_error("[ERROR] f_write failed: %d", res);
    exit(1);
  }
  log_info("write file success!");

  // 关闭文件
  res = f_close(&fp);
  if (res) {
    log_error("[ERROR] f_close failed: %d", res);
    exit(1);
  }
  log_info("close file success!");

  // 判断文件是否存在
  FILINFO fno;
  res = f_stat(filename, &fno);
  if (res) {
    log_error("[ERROR] f_stat failed: %d", res);
    exit(1);
  }
  log_info("f_stat test success!");
  // 输出文件信息
  log_info("File size: %d", fno.fsize);
  log_info(
      "Attributes: %c%c%c%c%c", (fno.fattrib & AM_DIR) ? 'D' : '-',
      (fno.fattrib & AM_RDO) ? 'R' : '-', (fno.fattrib & AM_HID) ? 'H' : '-',
      (fno.fattrib & AM_SYS) ? 'S' : '-', (fno.fattrib & AM_ARC) ? 'A' : '-');
  log_info("File name: %s", fno.fname);

  FSIZE_t sz = f_size(&fp);
  if (sz != str_len) {
    log_error("[ERROR] f_size failed: %d", res);
    exit(1);
  }
  log_info("f_size test success!");
}

void user_option_create_dir() {
  printf("请输入目录名：\n");
  char *s = uart_getline();
  FRESULT res = f_mkdir(s);
  if (res) {
    log_error("f_mkdir failed: %d", res);
    exit(1);
  }
  log_info("create dir success!");
}

void user_option_open_file() {
  printf("请输入文件名：\n");
  char *s = uart_getline();
  FIL fp;
  FRESULT res = f_open(&fp, s, FA_READ);
  if (res) {
    log_error("f_open failed: %d", res);
    exit(1);
  }
  log_info("open file success!");

  // 读取文件
  char buf[256];
  UINT bytes_read;
  f_read(&fp, buf, sizeof(buf), &bytes_read);
  log_info("read file success!");
  log_info("file content: %s", buf);

  // 关闭文件
  res = f_close(&fp);
  if (res) {
    log_error("[ERROR] f_close failed: %d", res);
    exit(1);
  }
  log_info("close file success!");
}

void user_option_list_dir() {
  printf("请输入目录名：\n");
  char *s = uart_getline();
  list_dir(s);
}

void user_option_list_dir_tree() { list_dir_tree("", 0); }

int user_filesystem(int argc, char **argv) {
  struct menu m;
  menu_init(&m, "文件系统操作演示项目", NULL);
  menu_add_item(&m, "创建文件", user_option_create_file, NULL);
  menu_add_item(&m, "创建目录", user_option_create_dir, NULL);
  menu_add_item(&m, "打开文件", user_option_open_file, NULL);
  menu_add_item(&m, "列举目录", user_option_list_dir, NULL);
  menu_add_item(&m, "递归列举目录", user_option_list_dir_tree, NULL);
  menu_add_item(&m, "一键创建复杂目录树", create_dir_tree, NULL);
  menu_loop(&m, true);
  return 0;
}