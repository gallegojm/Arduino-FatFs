/*
 * A class to wrap FatFs library from ChaN
 * Copyright (c) 2018 by Jean-Michel Gallego
 *
 * Use version R0.14 of FatFs
 *
 * Use SD library for Esp8266 for the low level device control
 * Use low level rutines of SdFat library with boards with others chips
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
 * You should have received a copy of the GNU General Public License,
 * If not, see <http://www.gnu.org/licenses/>.
 */
  
#include "FatFs.h"

#ifdef ESP8266
  Sd2Card card;
#else
  SdSpiCard card;
#endif

/*
extern "C" void sd_print( uint8_t a, uint32_t b )
{
//  Serial.println();
  Serial.print( "==> " );
  Serial.print( a );
  Serial.print( " " );
  Serial.println( b );
}
*/

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
  return 0;
}

extern "C" int sd_disk_read( uint8_t * buff, uint32_t sector, uint32_t count )
{
  uint8_t * b = buff;

  for( uint32_t n = 0; n < count; n ++ )
{
    if( card.readBlock( sector + n, b ) == 0 )
      return 1;
}
  return 0;
}

extern "C" int sd_disk_write( uint8_t * buff, uint32_t sector, uint32_t count )
{
  uint8_t * b = buff;
  
  for( int n = 0; n < count; n ++ )
    if( card.writeBlock( sector + n, b ) == 0 )
      return 1;
  return 0;
}

extern "C" int sd_disk_ioctl( uint8_t cmd, void * buff )
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
  
  return res;
}

extern "C" DWORD get_fattime( void )
{
  return ((DWORD)(FF_NORTC_YEAR - 1980) << 25 | (DWORD)FF_NORTC_MON << 21 | (DWORD)FF_NORTC_MDAY << 16);
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
//   csPin : SD card chip select pin
//   speed : SPI speed = SPI_HALF_SPEED (default), SPI_FULL_SPEED
// Return true if ok

#ifdef ESP8266
bool FatFsClass::begin( uint8_t csPin, uint32_t speed )
#else
bool FatFsClass::begin( uint8_t csPin, SPISettings spiSettings )
#endif
{
  ffs_result = 0;
#ifdef ESP8266
  if( ! card.init( speed, csPin ))
#else
  if( ! card.begin( &m_spi, csPin, spiSettings ))
#endif
    return false;
  ffs_result = f_mount( & ffs, "", 1 );
  return ffs_result == FR_OK;
}

// Return capacity of card in kBytes

int32_t FatFsClass::capacity()
{
  return ( ffs.n_fatent - 2 ) * ffs.csize >> 1; // >> 11;
}

// Return free space in kBytes

int32_t FatFsClass::free()
{
  uint32_t fre_clust;
  FATFS * fs;
  
  if( f_getfree( "0:", (DWORD*) & fre_clust, & fs ) != 0 )
    return -1;
  return fre_clust * ffs.csize >> 1; // >> 11;
}

// Return last error value
// See ff.h for a description of errors

uint8_t FatFsClass::error()
{
  return ffs_result;
}

// Make a directory
//   dirPath : absolute name of new directory
// Return true if ok

bool FatFsClass::mkdir( const char * path )
{
  ffs_result = f_mkdir( path );
  return ffs_result == FR_OK; // || res == FR_EXIST;
}

// Remove a directory (must be empty)
//   path : absolute name of directory to remove
// Return true if ok

bool FatFsClass::rmdir( const char * path )
{
  return remove( path );
}

// Remove a file
//   path : absolute name of file to remove
// Return true if ok

bool FatFsClass::remove( const char * path )
{
  ffs_result = f_unlink( path );
  return ffs_result == FR_OK;
}

// Rename a file or directory
//   oldName : old absolute name of file/directory to rename
//   newName : new absolute name
// Return true if ok

bool FatFsClass::rename( const char * oldName, const char * newName )
{
  // f_rename modify the value pointed by parameters oldName0 and newName0
  const char * oldName0 = oldName;
  const char * newName0 = newName;
  ffs_result = f_rename( oldName0, newName0 );
  return ffs_result == FR_OK;
}

// Return true if a file or directory exists
//   path : absolute name of file or directory

bool FatFsClass::exists( const char * path )
{
  if( strcmp( path, "/" ) == 0 )
    return true;
  return f_stat( path, NULL ) == FR_OK;
}

// Return true if a absolute name correspond to an existing directory

bool FatFsClass::isDir( const char * path )
{
  if( strcmp( path, "/" ) == 0 )
    return true;
  
  FILINFO finfo;
  return ( f_stat( path, & finfo ) == FR_OK ) &&
         ( finfo.fattrib & AM_DIR );
}

// Set time stamp of file or directory

bool FatFsClass::timeStamp( const char * path, uint16_t year, uint8_t month, uint8_t day,
                            uint8_t hour, uint8_t minute, uint8_t second )
{
  FILINFO finfo;
  
  finfo.fdate = ( year - 1980 ) << 9 | month << 5 | day;
  finfo.ftime = hour << 11 | minute << 5 | second >> 1;
  ffs_result = f_utime( path, & finfo );
  return ffs_result == FR_OK;
}

// Return date and time of last modification

bool FatFsClass::getFileModTime( const char * path, uint16_t * pdate, uint16_t * ptime )
{
  FILINFO finfo;
  
  if( f_stat( path, & finfo ) != FR_OK )
    return false;
  * pdate = finfo.fdate;
  * ptime = finfo.ftime;
  return true;
}

/* ===========================================================

                    DirFs functions

   =========================================================== */

// Open a directory
//   dirPath : absolute name of directory
// Return true if ok

bool DirFs::open( char * dirPath )
{
  ffs_result = f_opendir( & dir, dirPath );
  return ffs_result == FR_OK;
}

// Close the open directory

bool DirFs::close()
{
  ffs_result = f_closedir( & dir );
  return ffs_result == FR_OK;
}

// Read next directory entry
// Return false if end of directory is reached or an error had occurred

bool DirFs::nextFile()
{
  ffs_result = f_readdir( & dir, & finfo );
  return ffs_result == FR_OK && finfo.fname[0] != 0;
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
  return finfo.fname; 
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
  ffs_result = f_open( & ffile, fileName, mode );
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
  uint32_t lb, nwrt0, nwrt = 0;
  
  ffs_result = FR_OK;
  while( nwrt < lbuf && ffs_result == FR_OK )
  {
    nwrt0 = 0;
    lb = lbuf - nwrt;
    if( lb > FF_MIN_SS )
      lb = FF_MIN_SS;
    ffs_result = f_write( & ffile, ( buf + nwrt ), lb, (UINT*) & nwrt0 );
    nwrt += nwrt0;
  }
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
  uint32_t lb, nrd0, nrd;
  
  nrd = 0;
  do
  {
    nrd0 = 0;
    lb = lbuf - nrd;
    if( lb > FF_MIN_SS )
      lb = FF_MIN_SS;
    ffs_result = f_read( & ffile, ( buf + nrd ), lb, (UINT*) & nrd0 );
    nrd += nrd0;
  }
  while( nrd0 > 0 && nrd < lbuf && ffs_result == FR_OK );
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
  
  ffs_result = f_read( & ffile, & car, 1, (UINT*) & nrd );
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
  return f_size( & ffile );
}

FatFsClass FatFs;
