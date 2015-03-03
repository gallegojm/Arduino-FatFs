/*
 * Class FatLib by Jean-Michel Gallego
 * Copyright (c) 2015 by Jean-Michel Gallego
 *
 * FatLib allow you to easily switch between libraries SdFat and FatFs
 *   by setting identifier FAT_SYST at beginning of file FatLib.h
 
 *   SdFat is an Arduino library written by William Greiman
 *    that provides read/write access to FAT16/FAT32
 *    file systems on SD/SDHC flash cards.
 *   
 *   FatFs module is a generic FAT file system for
 *    small embedded systems developed by ChaN.
 *
 * You must install SdFat and FatFs in order to use FatLib
*/

// Set FAT_SYST to 0 to use library SdFat
// Set FAT_SYST to 1 to use library FatFs
#define FAT_SYST 1

#if FAT_SYST == 0
  #include <SdFat.h>

  #define _MAX_LFN 255

  #define FAT  sd
  #define FAT_FILE ExtFile
  #define FAT_DIR  ExtDir
#else
  #include <FatFs.h>

  #define FAT      FatFs
  #define FAT_FILE FileFs
  #define FAT_DIR  DirFs

  #define O_READ   FA_READ
  #define O_WRITE  FA_WRITE
  #define O_RDWR   FA_READ | FA_WRITE
  #define O_CREAT  FA_CREATE_ALWAYS
#endif

#ifndef FAT_LIB_H
#define FAT_LIB_H

#if FAT_SYST == 0

class ExtFat : public SdFat
{
public:
  int32_t  capacity() { return card()->cardSize() >> 11; };
  int32_t  free()     { return vol()->freeClusterCount() * vol()->blocksPerCluster() >> 11; };
  bool     timeStamp( char * path, uint16_t year, uint8_t month, uint8_t day,
                      uint8_t hour, uint8_t minute, uint8_t second );
  bool     getFileModTime( char * path, uint16_t * pdate, uint16_t * ptime );
};

extern ExtFat sd;

class ExtDir : public SdFile
{
public:
  bool     openDir( char * dirPath );
  bool     closeDir() { return close(); };
  bool     nextFile();
  bool     isDir()    { return isdir; };
  char *   fileName() { return lfn; };
  uint32_t fileSize() { return filesize; };
  uint16_t fileModDate() { return filelwd; };
  uint16_t fileModTime() { return filelwt; };

private:
  SdFile   curFile;
  char     lfn[ _MAX_LFN + 1 ];    // Buffer to store the LFN
  bool     isdir;
  uint32_t filesize;
  uint16_t filelwd;
  uint16_t filelwt;
};

class ExtFile : public SdFile
{
public:
  int      writeString( char * buf ) { return write( buf, strlen( buf )); }; 
  bool     writeChar( char car )     { return write( & car, 1 ); };
  int16_t  readString( char * buf, int len );
  char     readChar()                { return read(); };
  uint16_t readInt();
  uint16_t readHex();
};
#endif

#endif // FAT_LIB_H

