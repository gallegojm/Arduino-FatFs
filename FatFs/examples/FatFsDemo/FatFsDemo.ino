/*
  Test of a class to access the FatFs module
    on Arduino Due by Jean-Michel Gallego
  FatFs module is a generic FAT file system for
    small embedded systems developped by ChaN.

  Tested with Due, Mega2560 and Esp8266

  Wiring for Esp8266 and SD card reader:
  SD card   Esp8266
    MISO      #12
    MOSI      #13
    SCK       #14
    CS        #15 (Can be modified specifying value for SD_CS_PIN)
  
*/

#include <SPI.h>
#include <FatFs.h>

// Modify according to your hardware
// #define MY_SD_CS_PIN 4     // Chip Select for SD card reader with Ethernet shield
   #define MY_SD_CS_PIN 53    // Chip Select for SD card reader on LaRocola
// #define MY_SD_CS_PIN 15    // Default Chip Select for SD card reader on Esp8266

void setup()
{
  int res;

  // If other chips are connected to SPI bus, set to high the pin connected to their CS
  /*
  pinMode( 10, OUTPUT ); 
  digitalWrite( 10, HIGH );
  pinMode( 4, OUTPUT ); 
  digitalWrite( 4, HIGH );
  */

  Serial.begin( 115200 );
  Serial.println( "== Testing FatFs on Arduino ==" );
  Serial.println( "Press a key to start\n" );
  while( Serial.read() < 0 )
    delay( 1 );
  delay( 400 );

  // Mount SD card
#ifdef ESP8266
  res = FatFs.begin(); // Use default pin 15 for chip select
  //res = FatFs.begin( MY_SD_CS_PIN, SPI_FULL_SPEED );
#else
  res = FatFs.begin( MY_SD_CS_PIN, SD_SCK_MHZ(50));
  // res = FatFs.begin( MY_SD_CS_PIN, SPI_HALF_SPEED );
#endif
  printError( res, "Unable to mount SD card" );
  Serial.println( "SD card mounted" );

  // Show capacity and free space of SD card
  Serial.print( "Capacity of card:   " );
  Serial.print( FatFs.capacity());
  Serial.println( " MBytes" );
  Serial.print( "Free space on card: " );
  Serial.print( FatFs.free());
  Serial.println( " MBytes" );

  char * fileName0 = "LaRocola.cfg";
  Serial.print( fileName0 );
  Serial.println( FatFs.exists( fileName0 ) ? " exists" : " don't exists" );
  Serial.print( fileName0 );
  Serial.print( FatFs.isDir( fileName0 ) ? " is" : " is not" );
  Serial.println( " a directory" );

  char * fileName1 = "The Beatles";
  Serial.print( fileName1 );
  Serial.println( FatFs.exists( fileName1 ) ? " exists" : " don't exists" );
  Serial.print( fileName1 );
  Serial.print( FatFs.isDir( fileName1 ) ? " is" : " is not" );
  Serial.println( " a directory" );

  char * fileName2 = "Pepito";
  Serial.print( fileName2 );
  Serial.println( FatFs.exists( fileName2 ) ? " exists" : " don't exists" );

  // List root directory
  Serial.println( "List of directories and files in root:" );
  DirFs dir;
  if( dir.openDir( "/" ))
    while( dir.nextFile())
    {
      Serial.print( dir.fileName());
      Serial.print( "\t" );
      if( dir.isDir() )
        Serial.println( "(Directory)" );
      else
      {
        Serial.print( dir.fileSize());
        Serial.println( " Bytes" );
      }
    }

  Serial.println( "Second list of directories and files in root:" );
  if( dir.rewind())
    while( dir.nextFile())
      Serial.println( dir.fileName());

  // Create directory
  char * dirName = "/New Directory";
  Serial.print( "\nCreate directory '" );
  Serial.print( dirName );
  Serial.println( "'" );
  if( FatFs.exists( dirName ))
  {
    Serial.print( dirName );
    Serial.println( " already exists!" );
  }
  else
  {
    res = FatFs.mkdir( dirName );
    printError( res, "Error creating directory" );
  }
    Serial.print( dirName );
    Serial.println( " created!" );

  // Create a file in that directory
  FileFs file;
  char * fileName = "/New Directory/A new file.txt";
  Serial.print( "\nCreate file '" );
  Serial.print( fileName );
  Serial.println( "'" );
  res = file.open( fileName, FA_WRITE | FA_READ | FA_CREATE_ALWAYS );
  printError( res, "Error creating file" );
  
  // Writing text to a file and closing it
  Serial.println( "Write to file" );
  res = file.writeString( "Test d'écriture dans un fichier\r\n" );
  if( res >= 0 )
    res = file.writeString( "Testing writing to file\r\n" );
  if( res >= 0 )
    res = file.writeString( "Prueba de escritura en un archivo\r\n" );
  // Write next line using writeBuffer()
  if( res >= 0 )
  {
    char * ps1 = "Test di scrittura su file\r\n";
    uint32_t nwrite = file.write( ps1, strlen( ps1 ));
    res = nwrite == strlen( ps1 );
  }
  // Write last line byte per byte,  using writeChar()
  if( res >= 0 )
  {
    char * ps2 = "Testes de gravação em um arquivo\r\n";
    uint8_t pc = 0 ;
    while( res >= 0 && ps2[ pc ] != 0 )
      res = file.writeChar( ps2[ pc ++ ] );
  }
  printError( ( res >= 0 ), "Error writing to file" );

  // Read content of file
  Serial.print( "\nContent of '" );
  Serial.print( fileName );
  Serial.println( "' is:" );
  char line[ 64 ];
  file.seekSet( 0 ); // set cursor to beginning of file
  while( file.readString( line, sizeof( line )) >= 0 )
    Serial.println( line );

  // Size of the file
  Serial.print( "\nSize of file " );
  Serial.print( fileName );
  Serial.print( " : " );
  Serial.println( file.fileSize());

  // Close the file
  Serial.println( "\nClose the file" );
  res = file.close();
  printError( res, "Error closing file" );

  // Rename and move the file to root
  char * newName = "/TEST.TXT";
//  char * newName = "/Test d'écriture.txt";
  Serial.print( "\nRename file '" );
  Serial.print( fileName );
  Serial.print( "' to '" );
  Serial.print( newName );
  Serial.println( "'" );
  if( FatFs.exists( newName ))
    FatFs.remove( newName );
  res = FatFs.rename( fileName, newName );
  printError( res, "Error renaming file" );

  // Show content of directories
  Serial.print( "\nContent of '" );
  Serial.print( dirName );
  Serial.println( "' :" );
  if( dir.openDir( "/New Directory" ))
    while( dir.nextFile())
      Serial.println( dir.fileName());
  Serial.println( "\nContent of root :" );
  if( dir.openDir( "/" ))
    while( dir.nextFile())
      Serial.println( dir.fileName());

  // Open file for reading
  Serial.print( "\nOpen file '" );
  Serial.print( newName );
  Serial.println( "' :" );
  res = file.open( newName, FA_OPEN_EXISTING | FA_READ );
  printError( res, "Error opening file" );

  // Read content of file and close it
  Serial.print( "\nContent of '" );
  Serial.print( newName );
  Serial.println( "' is:" );
  // Read first line byte per byte, just to demostrate use of readChar()
  char c;
  do
  {
    c = file.readChar();
    Serial.print( c );
  }
  while( c != '\n' && c != -1 );
  // Read next lines with readString() and print length of lines
  int l;
  while( ( l = file.readString( line, sizeof( line ))) >= 0 )
  {
    Serial.print( line );
    Serial.print( " (Length is: " );
    Serial.print( l );
    Serial.println( ")" );
  }
  file.close();
    
  // Delete the file
  Serial.println( "\nDelete the file" );
  res = FatFs.remove( newName );
  printError( res, "Error deleting file" );

  // Delete the directory
  Serial.print( "\nDelete directory '" );
  Serial.print( dirName );
  Serial.println( "'" );
  res = FatFs.rmdir( dirName );
  printError( res, "Error deleting directory" );

  Serial.println( "\nTest ok!" );
}

void loop()
{
}

//    PRINT ERROR STRING & STOP EXECUTION

// if ok is false, print the string msg and enter a while loop for ever

void printError( int ok, char * msg )
{
  if( ok )
    return;
  Serial.print( msg );
  Serial.print( ": " );
  Serial.println( FatFs.error());
  while( true )
    delay( 1 );
}
