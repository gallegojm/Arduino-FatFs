
// Set FAT_SYST to 0 to use library SdFat
// Set FAT_SYST to 1 to use library FatFs
#define FAT_SYST 0

#if FAT_SYST == 0
  #include <SdFat.h>

  #define _MAX_LFN 255

  #define FAT  sd
  #define FAT_FILE ExtFile
  #define FAT_DIR  ExtDir
#else
  #include <SD.h>
  #include <FatFs.h>

  #define FAT      FatFs
  #define FAT_FILE FileFs
  #define FAT_DIR  DirFs

  #define O_READ   FA_READ
  #define O_WRITE  FA_WRITE
  #define O_RDWR   FA_READ | FA_WRITE
  #define O_CREATE FA_CREATE_ALWAYS
#endif

#ifndef FAT_LIB_H
#define FAT_LIB_H

#if FAT_SYST == 0

class ExtFat : public SdFat
{
public:
  int32_t  capacity() { return card()->cardSize() >> 11; };
  int32_t  free()     { return vol()->freeClusterCount() * vol()->blocksPerCluster() >> 11; };
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

private:
  SdFile   curFile;
  char     lfn[ _MAX_LFN + 1 ];    // Buffer to store the LFN
  bool     isdir;
  uint32_t filesize;
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
