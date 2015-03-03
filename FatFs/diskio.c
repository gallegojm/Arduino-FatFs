/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2014        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "diskio.h"		/* FatFs lower layer API */

// OJO // #include "usbdisk.h"	/* Example: USB drive control */
// OJO // #include "atadrive.h"	/* Example: ATA drive control */
// OJO // #include "sdcard.h"		/* Example: MMC/SDC contorl */

/* Definitions of physical drive number for each drive */
// OJO // #define ATA		0	/* Example: Map ATA drive to drive number 0 */
// OJO // #define MMC		1	/* Example: Map MMC/SD card to drive number 1 */
// OJO // #define USB		2	/* Example: Map USB drive to drive number 2 */

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status( BYTE pdrv ) // Physical drive nmuber to identify the drive
{
  return sd_status();
}

/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize( BYTE pdrv ) // Physical drive nmuber to identify the drive
{
  return sd_initialize();
}

/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read( BYTE pdrv,    // Physical drive nmuber to identify the drive
                   BYTE *buff,   // Data buffer to store read data
                   DWORD sector, // Sector address in LBA
                   UINT count )  // Number of sectors to read
{
  return sd_disk_read( buff, sector, count );
}

/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if _USE_WRITE
DRESULT disk_write( BYTE pdrv,        // Physical drive nmuber to identify the drive
                    const BYTE *buff, // Data to be written
                    DWORD sector,     // Sector address in LBA
                    UINT count )      // Number of sectors to write
{
  return sd_disk_write( buff, sector, count );
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

#if _USE_IOCTL
DRESULT disk_ioctl( BYTE pdrv,    // Physical drive nmuber (0..)
                    BYTE cmd,     // Control code
                    void *buff )  // Buffer to send/receive control data
{
  return sd_disk_ioctl( cmd );
}

#endif
