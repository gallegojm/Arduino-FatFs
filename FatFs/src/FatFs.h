/*
 * Classes to wrap FatFs library from ChaN
 * Copyright (c) 2018 by Jean-Michel Gallego
 *
 * Use version R0.12c of FatFs
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
 
#ifndef FATFS_H
#define FATFS_H

#include <Arduino.h>
#ifdef ESP8266
  #include <SD.h>
  #define SD_CS_PIN 15    // Chip Select for SD card reader on Esp8266
#else
  #include <SdFat.h>
#endif
#include "ff.h"
#include "diskio.h"

class FatFsClass
{
public:
  FatFsClass() {};
  
#ifdef ESP8266
  bool     begin( uint8_t csPin = SD_CS_PIN, uint32_t speed = SPI_FULL_SPEED );
#else
  bool     begin( uint8_t csPin, SPISettings spiSettings );
#endif
  int32_t  capacity();
  int32_t  free();
  uint8_t  error();
  
  bool     mkdir( const char * path );
  bool     rmdir( const char * path );
  bool     remove( const char * path );
  bool     rename( const char * oldName, const char * newName );
  bool     exists( const char * path );
  bool     isDir( const char * path );
  bool     timeStamp( const char * path, uint16_t year, uint8_t month, uint8_t day,
                      uint8_t hour, uint8_t minute, uint8_t second );
  bool     getFileModTime( const char * path, uint16_t * pdate, uint16_t * ptime );

private:
  FATFS    ffs;

#ifndef ESP8266
protected:
  SdFatSpiDriver m_spi;
#endif
};

extern FatFsClass FatFs;

class DirFs
{
public:
  DirFs()  {};
  ~DirFs() { f_closedir( & dir ); };
  
  bool     open( char * dirPath );
  bool     close();
  bool     nextFile();
  bool     rewind();
  bool     isDir();
  char *   fileName();
  uint32_t fileSize();
  uint16_t fileModDate();
  uint16_t fileModTime();

private:
  FILINFO  finfo;
  DIR      dir;
};

class FileFs
{
public:
  FileFs() {};
  
  bool     open( char * fileName, uint8_t mode = FA_OPEN_EXISTING );
  bool     close();
  
  uint32_t write( void * buf, uint32_t lbuf );
  int      writeString( char * str );
  bool     writeChar( char car );
  
  uint32_t read( void * buf, uint32_t lbuf );
  int16_t  readString( char * buf, int len );
  char     readChar();
  uint16_t readInt();
  uint16_t readHex();

  uint32_t curPosition();
  bool     seekSet( uint32_t cur );

  uint32_t fileSize();
  
private:
  FIL      ffile;
};

// Return true if char c is allowed in a long file name

inline bool legalChar( char c )
{
  if( c == '"' || c == '*' || c == '?' || c == ':' || 
      c == '<' || c == '>' || c == '|' )
    return false;
  return true;
}

#endif // FATFS_H
