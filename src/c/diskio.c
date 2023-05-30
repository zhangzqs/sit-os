/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h" /* Declarations of disk functions */

#include "ff.h" /* Obtains integer types */

/* Definitions of physical drive number for each drive */
#define DEV_RAM 0 /* Example: Map Ramdisk to physical drive 0 */

// 开辟10M的内存空间作为ramdisk
#define SECTOR_SIZE 512
#define SECTOR_COUNT 2000
#define RAMDISK_SIZE (SECTOR_SIZE * SECTOR_COUNT)

static BYTE ramdisk[RAMDISK_SIZE];

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status(BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
  switch (pdrv) {
    case DEV_RAM:
      return 0;
  }
  return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize(
    BYTE pdrv /* Physical drive nmuber to identify the drive */
) {
  switch (pdrv) {
    case DEV_RAM:
      return 0;
  }
  return STA_NOINIT;
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read(BYTE pdrv,  /* Physical drive nmuber to identify the drive */
                  BYTE *buff, /* Data buffer to store read data */
                  LBA_t sector, /* Start sector in LBA */
                  UINT count    /* Number of sectors to read */
) {
  switch (pdrv) {
    case DEV_RAM: {
      // 将ramdisk中，偏移量为sector * SECTOR_SIZE的数据，拷贝到buff中，
      // 拷贝的数据长度为count*512
      BYTE *dst = buff;
      const BYTE *src = ramdisk + sector * SECTOR_SIZE;
      UINT n = count * SECTOR_SIZE;
      while (n--) {
        *dst++ = *src++;
      }

      return RES_OK;
    }
  }

  return RES_PARERR;
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write(BYTE pdrv, /* Physical drive nmuber to identify the drive */
                   const BYTE *buff, /* Data to be written */
                   LBA_t sector,     /* Start sector in LBA */
                   UINT count        /* Number of sectors to write */
) {
  switch (pdrv) {
    case DEV_RAM: {
      // 将buff中的数据，拷贝到ramdisk中，偏移量为sector * SECTOR_SIZE的位置
      // 拷贝的数据长度为count*512
      BYTE *dst = ramdisk + sector * SECTOR_SIZE;
      const BYTE *src = buff;
      UINT n = count * SECTOR_SIZE;
      while (n--) {
        *dst++ = *src++;
      }
      return RES_OK;
    }
  }

  return RES_PARERR;
}

#endif

/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl(BYTE pdrv, /* Physical drive nmuber (0..) */
                   BYTE cmd,  /* Control code */
                   void *buff /* Buffer to send/receive control data */
) {
  switch (pdrv) {
    case DEV_RAM: {
      switch (cmd) {
        case GET_SECTOR_COUNT:
          *(DWORD *)buff = SECTOR_COUNT;
          break;
        case GET_SECTOR_SIZE:
          *(DWORD *)buff = SECTOR_SIZE;
          break;
      }
      return RES_OK;
    }
  }

  return RES_PARERR;
}
