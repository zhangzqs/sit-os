#include "ff.h"
#include "diskio.h"
#include "log.h"

static void exit(int code) { panic("exit(%d)", code); }
static void assert(bool cond) {
  if (!cond) {
    panic("assertion failed");
  }
}

// 磁盘分区
PARTITION VolToPart[FF_VOLUMES] = {
    // 物理磁盘号, 分区号
    {0, 1},
    {0, 2},
};

static void test_disk(BYTE pdrv) {
  BYTE write_buff[1024];
  for (int i = 0; i < 1024; i++) {
    write_buff[i] = (i + 1) % 256;
  }
  disk_write(0, write_buff, 0, 2);

  BYTE read_buff[512];

  disk_read(0, read_buff, 0, 1);
  for (int i = 0; i < 512; i++) {
    if (read_buff[i] != (i + 1) % 256) {
      log_error("[ERROR] read_buff[%d] = %d", i, read_buff[i]);
      exit(1);
    }
  }

  disk_read(0, read_buff, 1, 1);
  for (int i = 0; i < 512; i++) {
    if (read_buff[i] != (i + 512 + 1) % 256) {
      log_error("[ERROR] read_buff[%d] = %d", i, read_buff[i]);
      exit(1);
    }
  }
  log_info("disk test passed!");
}

// 磁盘分区
static void make_partition(BYTE pdrv) {
  // 工作缓冲区
  BYTE work[FF_MAX_SS];

  // 磁盘分为两个大小相等的分区
  LBA_t plist[] = {50, 50, 0};
  log_info("make partition...");
  FRESULT res = f_fdisk(pdrv, plist, work);
  if (res) {
    log_error("[ERROR] f_fdisk failed: %d", res);
    exit(1);
  }

  disk_read(pdrv, work, 0, 1);
  // 判断分MBR分区表是否正确
  if (work[510] != 0x55 || work[511] != 0xAA) {
    log_error("[ERROR] MBR partition table is incorrect!");
    exit(1);
  }
  log_info("MBR partition table is correct!");
}

static void format_volume() {
  // 工作缓冲区
  BYTE work[FF_MAX_SS];

  log_info("format volume...");

  FRESULT res = f_mkfs("0:", 0, work, sizeof(work));
  if (res) {
    log_error("[ERROR] f_mkfs failed: %d", res);
    exit(1);
  }
  log_info("format volume success!");
}

void make_sure_init() {
  static bool inited = false;
  static FATFS fs;

  if (inited) {
    return;
  }  
  const BYTE pdrv = 0;

  // 磁盘初始化
  disk_initialize(pdrv);
  // 磁盘分区
  make_partition(pdrv);
  // 格式化磁盘
  format_volume();
  // 挂载磁盘
  assert(f_mount(&fs, "0:", 0) == FR_OK);

  inited = true;
}