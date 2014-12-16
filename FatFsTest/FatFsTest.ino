/*
    Test of a wrapper to FatFs on Arduino

  Copyright (C) 2014, Jean-Michel Gallego, all right reserved.

    FatFs module is a generic FAT file system module
  for small embedded systems developped by ChaN.

    This programm is free software and there is NO WARRANTY.
  No restriction on use. You can use, modify and redistribute it for
  personal, non-profit or commercial products UNDER YOUR RESPONSIBILITY.
  Redistributions of source code must retain the above copyright notice.

    Tested on Arduino Due and Arduino Mega 2560 with IDE 1.5.5 and 1.5.8
    Remember to modify value of SD_CS_PIN according to your hardware
*/

#include <Streaming.h>
#include <SPI.h>
#include <SD.h>
#include "ff.h"
#include "diskio.h"

FATFS FatFs;
Sd2Card card;

// #define SD_CS_PIN 9    // Chip Select for SD card reader on LaRocola
#define SD_CS_PIN 4    // Chip Select for SD card reader with Ethernet shield

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
  return card.init( SPI_HALF_SPEED, SD_CS_PIN ) > 0 ? 0 : 1;
}

extern "C" int sd_disk_read( uint8_t * buff, uint32_t sector, uint32_t count )
{
  uint8_t * b = buff;
  
  for( int n = 0; n < count; n ++ )
    if( card.readBlock( sector + n, b ) == 0 )
      return 1;
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

extern "C" int sd_disk_ioctl( uint8_t cmd )
{
  DRESULT res = RES_ERROR;
  
  switch( cmd )
  {
    case CTRL_SYNC :   // Make sure that data has been written  
      res = RES_OK;
      break;  

    default:  
      res = RES_PARERR;  
  }
}

extern "C" uint32_t get_fattime( void )
{
  return 0;
}

void printError( int err, char * msg )
{
  if( err == 0 )
    return;
  Serial << msg << endl;
  while( 1 );
}

void setup()
{
  int res;

  delay( 1000 );

  Serial.begin( 9600 );
  Serial << "Testing FatFs on Arduino" << endl << endl;

  // If other chips connected to SPI bus, set to high the pin connected to their CS
  pinMode( 10, OUTPUT ); 
  digitalWrite( 10, HIGH );
  pinMode( 4, OUTPUT ); 
  digitalWrite( 4, HIGH );

  // Mount SD card
  res = f_mount( & FatFs, "", 1 );
  printError( res, "Unable to mount SD card" );
  Serial << "SD card mounted" << endl;

  // Show capacity and free space of SD card
  uint32_t fre_clust, fre_sect, tot_sect;
  FATFS * fs;
  // res = f_getfree( (const TCHAR*) "0:", & fre_clust, &fs );
  res = f_getfree( "0:", & fre_clust, &fs );
  printError( res, "Error calculating free clusters number" );
  tot_sect = ( FatFs.n_fatent - 2 ) * FatFs.csize;
  fre_sect = fre_clust * FatFs.csize;
  Serial << "Capacity of card: " << tot_sect / 2 << " kB" << endl;
  Serial << "Free space on card: " << fre_sect / 2 << " kB" << endl;
  
  // List root directory
  FILINFO fno;
  DIR dir;
  int i;
  static char lfn[_MAX_LFN + 1];    // Buffer to store the LFN
  fno.lfname = lfn;
  fno.lfsize = sizeof lfn;
  //res = f_opendir( & dir, (const TCHAR*) "/" );    // Open the directory
  res = f_opendir( & dir, "/" );    // Open the directory
  printError( res, "Error opening root directory" );
  Serial << endl << "List of directories and files in root:" << endl;
  while( 1 )
  {
    res = f_readdir( & dir, & fno );
    if( res != 0 || fno.fname[0] == 0 )
      break;
    if( fno.fname[0] == '.' )
      continue;
    Serial << fno.fname << "\t";
    if( * fno.lfname )
      Serial << "  " << fno.lfname;
    Serial << endl;
  }
  f_closedir( & dir );
  
  // Create directory
  char * absDirName = "/New Directory";
  char * relDirName = absDirName + 1;
  Serial << endl << "Create directory \"" << relDirName << "\"" << endl;
  if( f_stat( relDirName, & fno ) == FR_NO_FILE )
  {
    res = f_mkdir( relDirName );
    printError( res, "Error creating directory" );
  }
  
  // Move to that directory
  Serial << endl << "Move to directory \"" << relDirName << "\"" << endl;
  res = f_chdir( absDirName );
  printError( res, "Error moving to new directory" );

  // Creating a file in that directory
  FIL file;
  char * fileName = "A new file.txt";
  Serial << endl << "Create file \"" << fileName << "\" in \"" << absDirName << "\"" << endl;
  res = f_open( & file, fileName, FA_WRITE | FA_OPEN_ALWAYS );
  printError( res, "Error creating file" );
  
  // Writing text to file and closing it
  res = f_puts( "Test d'ecriture dans un fichier\n\r", & file );
  if( res >= 0 )
    res = f_puts( "Testing writing to file\n\r", & file );
  if( res >= 0 )
    res = f_puts( "Prueba de escritura en el archivo\n\r", & file );
  printError( ( res == -1 ), "Error writing to file" );
  res = f_close( & file );
  printError( res, "Error closing file" );

  Serial << endl << "List of directories and files in current directory" << endl;
  char * fn;
  f_opendir( & dir, "" );
  while( 1 )
  {
    res = f_readdir( & dir, & fno );
    if( res != 0 || fno.fname[0] == 0 )
      break;
    fn = *fno.lfname ? fno.lfname : fno.fname;
    Serial << fn << endl;
  }
  f_closedir( & dir );

  // Open and display a file
  char line[ 64 ];
  Serial << endl << "Open and display file \"" << fileName << "\"" << endl;
  res = f_open( & file, fileName, FA_READ );
  printError( res, "Error opening file" );
  while( f_gets( line, sizeof( line ), & file ))
    Serial << line;
  f_close( & file );
  
  // Deleting a file
  Serial << endl << "Delete file \"" << fileName << "\"" << endl;
  res = f_unlink( fileName );
  printError( res, "Error deleting file" );

  // Deleting a directory
  Serial << endl << "Delete directory \"" << absDirName << "\"" << endl;
  res = f_chdir( "/" );
  res = f_unlink( absDirName );
  printError( res, "Error deleting directory" );

  Serial << endl << "List of directories and files in root:" << endl;
  f_opendir( & dir, "/" );
  while( 1 )
  {
    res = f_readdir( & dir, & fno );
    if( res != 0 || fno.fname[0] == 0 )
      break;
    fn = *fno.lfname ? fno.lfname : fno.fname;
    Serial << fn << endl;
  }
  f_closedir( & dir );
}

void loop()
{
}
