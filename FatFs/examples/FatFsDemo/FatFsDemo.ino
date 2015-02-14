/*
  Test of a class to access the FatFs module
    on Arduino Due by Jean-Michel Gallego
  FatFs module is a generic FAT file system for
    small embedded systems developped by ChaN.
*/

#include <Streaming.h>
#include <SPI.h>
#include <FatFs.h>

// Modify according to your hardware
// #define SD_CS_PIN 9    // Chip Select for SD card reader on LaRocola
#define SD_CS_PIN 4    // Chip Select for SD card reader with Ethernet shield

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

  Serial.begin( 9600 );
  Serial << "Testing FatFs on Arduino" << endl
         << "Press a key to start" << endl << endl;
  while( Serial.read() < 0 )
    ;
  delay( 400 );

  // Mount SD card
  res = FatFs.begin( SD_CS_PIN, SPI_HALF_SPEED );
  printError( res, "Unable to mount SD card" );
  Serial << "SD card mounted" << endl;

  // Show capacity and free space of SD card
  Serial << "Capacity of card:   " << FatFs.capacity() << " MBytes" << endl;
  Serial << "Free space on card: " << FatFs.free() << " MBytes" << endl;

  // List root directory
  Serial << endl << "List of directories and files in root:" << endl;
  DirFs dir;
  if( dir.openDir( "/" ))
    while( dir.nextFile())
    {
      Serial << dir.fileName() << "\t";
      if( dir.isDir() )
        Serial << "(Directory)" << endl;
      else
        Serial << dir.fileSize() << " Bytes" << endl;
    }
  
  // Create directory
  char * dirName = "/New Directory";
  Serial << endl << "Create directory '" << dirName << "'" << endl;
  if( FatFs.exists( dirName ))
    Serial << dirName << " already exists!" << endl;
  else
  {
    res = FatFs.mkdir( dirName );
    printError( res, "Error creating directory" );
  }

  // Create a file in that directory
  FileFs file;
  char * fileName = "/New Directory/A new file.txt";
  Serial << endl << "Create file '" << fileName <<"'" << endl;
  res = file.open( fileName, FA_WRITE | FA_READ | FA_CREATE_ALWAYS );
  printError( res, "Error creating file" );
  
  // Writing text to a file and closing it
  Serial << endl << "Write to file" << endl;
  res = file.writeString( "Test d'ecriture dans un fichier\r\n" );
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
    char * ps2 = "Testes de gravacao em um arquivo\r\n";
    uint8_t pc = 0 ;
    while( res >= 0 && ps2[ pc ] != 0 )
      res = file.writeChar( ps2[ pc ++ ] );
  }
  printError( ( res >= 0 ), "Error writing to file" );

  // Read content of file
  Serial << endl << "Content of '" << fileName << "' is:" << endl;
  char line[ 64 ];
  file.seekSet( 0 ); // set cursor to beginning of file
  while( file.readString( line, sizeof( line )) >= 0 )
    Serial << line << endl;

  // Close the file
  Serial << endl << "Close the file" << endl;
  res = file.close();
  printError( res, "Error closing file" );

  // Rename and move the file to roor
  char * newName = "/TEST.TXT";
  Serial << endl << "Rename file '" << fileName << "' to '" << newName << "'" << endl;
  if( FatFs.exists( newName ))
    FatFs.remove( newName );
  res = FatFs.rename( fileName, newName );
  printError( res, "Error renaming file" );

  // Show content of directories
  Serial << endl << "Content of '" << dirName << "' :" << endl;
  if( dir.openDir( "/New Directory" ))
    while( dir.nextFile())
      Serial << dir.fileName() << endl;
  Serial << endl << "Content of root :" << endl;
  if( dir.openDir( "/" ))
    while( dir.nextFile())
      Serial << dir.fileName() << endl;

  // Open file for reading
  Serial << endl << "Open file '" << newName << endl;
  res = file.open( newName, FA_OPEN_EXISTING | FA_READ );
  printError( res, "Error opening file" );

  // Read content of file and close it
  Serial << endl << "Content of '" << newName << "' is:" << endl;
  // Read first line byte per byte, just to demostrate use of readChar()
  char c;
  do
  {
    c = file.readChar();
    Serial << c;
  }
  while( c != '\n' && c != -1 );
  // Read next lines with readString() and print length of lines
  int l;
  while( ( l = file.readString( line, sizeof( line ))) >= 0 )
    Serial << line << " (Length is: " << l << ")" << endl;
  file.close();
    
  // Delete the file
  Serial << endl << "Delete the file" << endl;
//  res = FatFs.remove( path );
  res = FatFs.remove( newName );
  printError( res, "Error deleting file" );

  // Delete the directory
  Serial << endl << "Delete diectory '" << dirName << "'" << endl;
  res = FatFs.rmdir( dirName );
  printError( res, "Error deleting directory" );

  Serial << endl;
  Serial << "Test ok!" << endl;
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
  Serial << msg << ": " << FatFs.error() << endl;
  while( 1 );
}

