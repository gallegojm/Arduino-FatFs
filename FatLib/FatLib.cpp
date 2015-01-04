#include "FatLib.h"
#if FAT_SYST == 0

ExtFat sd;

/* ===========================================================

                    ExtDir functions

   =========================================================== */

// Open a directory
//   dirPath : absolute name of directory
// Return true if ok

bool ExtDir::openDir( char * dirPath )
{
  if( * dirPath != 0 )
    return open( dirPath );
  else
    return open( "/" );
}

// Read next directory entry
// Return false if end of directory is reached or an error had occurred

bool ExtDir::nextFile()
{
  if( ! curFile.openNext( this, O_READ ))
    return false;
  curFile.getFilename( lfn, _MAX_LFN );
  isdir = curFile.isSubDir();
  filesize = curFile.fileSize();
  curFile.close();
  return true;
}

/* ===========================================================

                    ExtFile functions

   =========================================================== */

// Read a string from the file
//   str : read buffer
//   len : size of read buffer
// Return number of characters read or -1 if an error occurs

int16_t ExtFile::readString( char * str, int len )
{
  if( fgets( str, len ) < 0 )
    return -1;
  uint16_t lstr = strlen( str ) - 1;
  while( lstr > 0 && ( str[ lstr ] == '\n' || str[ lstr ] == '\r' ))
    str[ lstr -- ] = 0;
  return lstr;
}

// Read next literal integer from file
// Return value of integer

uint16_t ExtFile::readInt()
{
  uint16_t i = 0;
  char c;
  // skip characters they are not integer
  do
    c = read();
  while( c >= 0 && ! isdigit( c ));
  while( isdigit( c ))
  {
    i = 10 * i + c - '0';
    c = read();
  }
  return i;
}

// Read next literal hexadecinal integer from file
// Return value of integer

uint16_t ExtFile::readHex()
{
  uint16_t i = 0;
  char c;
  // skip characters they are not hexadecimal
  do
    c = read();
  while( c >= 0 && ! isxdigit( c ));
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
    c = read();
  }
  return i;
}

#endif
