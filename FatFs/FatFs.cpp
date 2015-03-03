/*
 * A class to wrap FatFs library from ChaN
 * Copyright (c) 2014 by Jean-Michel Gallego
 *
 * Use version R0.10c of FatFs updated at November 26, 2014
*
 * This Library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Arduino SdSpiCard Library.  If not, see
 * <http://www.gnu.org/licenses/>.
 */
 
#include "FatFs.h"

FatFsCard card;

extern "C" int sd_status()
{
  if( card.type() == 0 )
    return STA_NOINIT;
  return 0; // card.errorData();
}

// card.init return 1 if initialisation is successfull.
// sd_initialize must return 0 when success

extern "C" int sd_initialize()
{
  return 0; // card.init( SPI_HALF_SPEED, SD_CS_PIN ) > 0 ? 0 : 1;
}

extern "C" int sd_disk_read( uint8_t * buff, uint32_t sector, uint32_t count )
{
  return card.readBlocks( sector, buff, count ) ? 0 : 1;
}

extern "C" int sd_disk_write( uint8_t * buff, uint32_t sector, uint32_t count )
{
  return card.writeBlocks( sector, buff, count ) ? 0 : 1;
}

extern "C" int sd_disk_ioctl( uint8_t cmd )
{
  DRESULT res = RES_ERROR;
  
  switch( cmd )
  {
    case CTRL_SYNC :   // Make sure that data has been written  
      res = RES_OK;
      // res = spiRec() == 0XFF ? RES_OK : RES_NOTRDY ;
      // res = card.waitNotBusy( SD_WRITE_TIMEOUT ) ? RES_OK : RES_NOTRDY ;  
      break;  

    default:  
      res = RES_PARERR;  
  }
}

extern "C" uint32_t get_fattime( void )
{
  return 0;
}

extern "C" void* ff_memalloc (UINT msize)
{
  return malloc( msize );
}

extern "C" void ff_memfree (void* mblock)
{
  free( mblock );
}

/* ===========================================================

                    FatFsClass functions

   =========================================================== */

uint8_t ffs_result;

// Initialize SD card and file system
//   chipSelectPin : SD card chip select pin
//   divisor : SPI divisor = SPI_HALF_SPEED (default), SPI_FULL_SPEED
// Return true if ok

bool FatFsClass::begin( uint8_t csPin, uint8_t sckDiv )
{
  ffs_result = 0;
  if( ! card.begin( csPin, sckDiv ))
    return false;
  ffs_result = f_mount( & ffs, "", 1 );
  return ffs_result == 0;
}

// Return capacity of card in Megabytes

int32_t FatFsClass::capacity()
{
  return ( ffs.n_fatent - 2 ) * ffs.csize >> 11;
}

// Return free space in Megabytes

int32_t FatFsClass::free()
{
  uint32_t fre_clust;
  FATFS * fs;
  
  if( f_getfree( "0:", & fre_clust, &fs ) != 0 )
    return -1;
  return fre_clust * ffs.csize >> 11;
}

// Return last error value
// See ff.h for a description of errors

uint8_t FatFsClass::error()
{
  return ffs_result;
}

// Make a directory
//   path : absolute name of new directory
// Return true if ok

bool FatFsClass::mkdir( char * path )
{
  char * path0 = path;
  ffs_result = f_mkdir( path0 );
  return ffs_result == FR_OK; // || res == FR_EXIST;
}

// Remove a directory (must be empty)
//   path : absolute name of directory to remove
// Return true if ok

bool FatFsClass::rmdir( char * path )
{
  return remove( path );
}

// Remove a file
//   path : absolute name of file to remove
// Return true if ok

bool FatFsClass::remove( char * path )
{
  char * path0 = path;
  ffs_result = f_unlink( path0 );
  return ffs_result == FR_OK;
}

// Rename a file or directory
//   oldName : old absolute name of file/directory to rename
//   newName : new absolute name
// Return true if ok

bool FatFsClass::rename( char * oldName, char * newName )
{
  char * oldName0 = oldName;
  char * newName0 = newName;
  ffs_result = f_rename( oldName0, newName0 );
  return ffs_result == FR_OK;
}

// Return true if a file or directory exists
//   path : absolute name of file or directory

bool FatFsClass::exists( char * path )
{
  if( strcmp( path, "/" ) == 0 )
    return true;
  
  FILINFO finfo;
  char    lfn[ _MAX_LFN + 1 ];    // Buffer to store the LFN
  char *  path0 = path;
  
  finfo.lfname = lfn;
  finfo.lfsize = _MAX_LFN + 1;
  return f_stat( path0, & finfo ) == FR_OK;
}

// Return true if a absolute name correspond to an existing directory

bool FatFsClass::isDir( char * path )
{
  if( strcmp( path, "/" ) == 0 )
    return true;
  
  FILINFO finfo;
  char    lfn[ _MAX_LFN + 1 ];    // Buffer to store the LFN
  char *  path0 = path;
  
  finfo.lfname = lfn;
  finfo.lfsize = _MAX_LFN + 1;
  return ( f_stat( path0, & finfo ) == FR_OK ) &&
         ( finfo.fattrib & AM_DIR );
}

// Set time stamp of file or directory

bool FatFsClass::timeStamp( char * path, uint16_t year, uint8_t month, uint8_t day,
                            uint8_t hour, uint8_t minute, uint8_t second )
{
  FILINFO fno;
  
  fno.fdate = ( year - 1980 ) << 9 | month << 5 | day;
  fno.ftime = hour << 11 | minute << 5 | second >> 1;
  return ffs_result == f_utime( path, &fno );
}

// Return date and time of last modification

bool FatFsClass::getFileModTime( char * path, uint16_t * pdate, uint16_t * ptime )
{
  FILINFO finfo;
  finfo.lfname = NULL;
  
  if( f_stat( path, & finfo ) != FR_OK )
    return false;
  * pdate = finfo.fdate;
  * ptime = finfo.ftime;
  return true;
}

/* ===========================================================

                    DirFs functions

   =========================================================== */

DirFs::DirFs()
{
  finfo.lfname = lfn;
  finfo.lfsize = _MAX_LFN + 1;
}

DirFs::~DirFs()
{
  f_closedir( & dir );
}

// Open a directory
//   dirPath : absolute name of directory
// Return true if ok

bool DirFs::openDir( char * dirPath )
{
  char * dirPath0 = dirPath;
  ffs_result = f_opendir( & dir, dirPath0 );
  return ffs_result == FR_OK;
}

// Close the open directory

bool DirFs::closeDir()
{
  ffs_result = f_closedir( & dir );
  return ffs_result == FR_OK;
}

// Read next directory entry
// Return false if end of directory is reached or an error had occurred

bool DirFs::nextFile()
{
  ffs_result = f_readdir( & dir, & finfo );
  return ffs_result == 0 && finfo.fname[0] != 0;
}

// Rewind the read index 

bool DirFs::rewind()
{
  ffs_result = f_readdir ( & dir, NULL );
  return ffs_result == FR_OK;
}

// Return true if the pointed entry is a directory 

bool DirFs::isDir()
{
  return finfo.fattrib & AM_DIR;
}

// Return a pointer to a string containing the name of the pointed entry

char * DirFs::fileName()
{
  return finfo.lfname[0] != 0 ? finfo.lfname : finfo.fname; 
}

// Return the size of the pointed entry

uint32_t DirFs::fileSize()
{
  return finfo.fsize;
}

// Return date of last modification

uint16_t DirFs::fileModDate()
{
  return finfo.fdate;
}

// Return time of last modification

uint16_t DirFs::fileModTime()
{
  return finfo.ftime;
}

/* ===========================================================

                    FileFs functions

   =========================================================== */

// Open a file
//   fileName : absolute name of the file to open
//   mode : specifies the type of access and open method for the file
//          (see ff.h for a description of possible values)
// Return true if ok
   
bool FileFs::open( char * fileName, uint8_t mode )
{
  char * fileName0 = fileName;
  ffs_result = f_open( & ffile, fileName0, mode );
  return ffs_result == FR_OK;
}

// Close the file
// Return true if ok

bool FileFs::close()
{
  ffs_result =  f_close( & ffile );
  return ffs_result == FR_OK;
}

// Writes data to the file
//   buf : pointer to the data to be written
//   lbuf : number of bytes to write
// Return number of bytes written

uint32_t FileFs::write( void * buf, uint32_t lbuf )
{
  uint32_t nwrt;
  
  ffs_result = f_write( & ffile, buf, lbuf, & nwrt );
  return nwrt;
}

// Write a string to the file
//   str : string to write
// Return number of characters written or -1 if an error occurs

int FileFs::writeString( char * str )
{
  return f_puts( str, & ffile );
}

// Write a character to the file
//   car : character to write
// Return true if ok

bool FileFs::writeChar( char car )
{
  return f_putc( car, & ffile ) == 1;
}

// Read data from the file
//   buf : pointer to buffer where to store read data
//   lbuf : number of bytes to read
// Return number of read bytes

uint32_t FileFs::read( void * buf, uint32_t lbuf )
{
  uint32_t nrd;
  
  ffs_result = f_read( & ffile, buf, lbuf, & nrd );
  return nrd;
}

// Read a string from the file
//   str : read buffer
//   len : size of read buffer
// Return number of characters read or -1 if an error occurs

int16_t FileFs::readString( char * str, int len )
{
  str = f_gets( str, len, & ffile );
  if( str == NULL )
    return -1;
  uint16_t lstr = strlen( str ) - 1;
  while( lstr > 0 && ( str[ lstr ] == '\n' || str[ lstr ] == '\r' ))
    str[ lstr -- ] = 0;
  return lstr;
}

// Read a character from the file
// Return read char or -1 if an error occurs
// In case of -1 returned, must call FatFs.error() to know
//   if this is an error or null character

char FileFs::readChar()
{
  char     car;
  uint32_t nrd;
  
  ffs_result = f_read( & ffile, & car, 1, & nrd );
  return nrd == 1 ? car : -1;
}

// Read next literal integer from file
// Return value of integer
// You must call FatFs.error() to know if an error has occurred

uint16_t FileFs::readInt()
{
  uint16_t i = 0;
  char c;
  // skip characters they are not integer
  do
    c = readChar();
  while( c != -1 && ! isdigit( c ));
  while( isdigit( c ))
  {
    i = 10 * i + c - '0';
    c = readChar();
  }
  return i;
}

// Read next literal hexadecinal integer from file
// Return value of integer
// You must call FatFs.error() to know if an error has occurred

uint16_t FileFs::readHex()
{
  uint16_t i = 0;
  char c;
  // skip characters they are not hexadecimal
  do
    c = readChar();
  while( c != -1 && ! isxdigit( c ));
  while( isxdigit( c ))
  {
    i = 16 * i + c;
    if( isdigit( c ))
      i -= '0';
    else
    {
      i += 10;
      if( c < 'F' )
        i -= 'A';
      else
        i -= 'a';
    }
    c = readChar();
  }
  return i;
}

// Return the current read/write pointer of a file

uint32_t FileFs::curPosition()
{
  return f_tell( & ffile );
}

// Moves the file read/write pointer
//   cur : new file pointer value
// Return if ok
// In case cur is greater than file size and file is opened in write mode,
//   size of file is expanded

bool FileFs::seekSet( uint32_t cur )
{
  ffs_result = f_lseek( & ffile, cur );
  return ffs_result == FR_OK;
}

// Return size of file

uint32_t FileFs::fileSize()
{
  return ffile.fsize;
}


FatFsClass FatFs;
