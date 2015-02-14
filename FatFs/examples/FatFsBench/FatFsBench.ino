/*
 * This program is a simple binary write/read benchmark.
 * Adapted from a program writen by William Greiman
 */

#include <Streaming.h>
#include <SPI.h>
#include <FatFs.h>

// SD chip select pin
// const uint8_t SD_CS_PIN = 9;
const uint8_t SD_CS_PIN = 4;

// Constants for test configuration
const size_t BUF_SIZE = 512;       // Size of read/write
const uint32_t FILE_SIZE_MB = 5;   // File size in MB where MB = 1,000,000 bytes
const uint8_t WRITE_COUNT = 1;     // Write pass count
const uint8_t READ_COUNT = 1;       // Read pass count

// File size in bytes.
const uint32_t FILE_SIZE = 1000000UL*FILE_SIZE_MB;
// Buffer for read/write operations
uint8_t buf[BUF_SIZE];
// test file
FileFs file;

//------------------------------------------------------------------------------
void setup()
{
  delay( 1000 );

  Serial.begin( 9600 );
  Serial << "Testing FatFs on Arduino Due" << endl;
}
//------------------------------------------------------------------------------
void loop()
{
  int res;
  float s;
  uint32_t t;
  uint32_t maxLatency;
  uint32_t minLatency;
  uint32_t totalLatency;

  // discard any input
  while( Serial.read() >= 0 )
    ;

  Serial << "Type any character to start" << endl;
  while( Serial.read() <= 0) ;
  delay( 400 );  // catch Due reset problem

  freeRam();

  res = FatFs.begin( SD_CS_PIN, SPI_HALF_SPEED );
  printError( res, "Unable to mount SD card" );
  Serial << "SD card mounted" << endl
         << "Card size:  " << FatFs.capacity() << " MB" << endl
         << "Free space: " << FatFs.free() << " MB" << endl << endl; 

  // fill buf with known data
  for( uint16_t i = 0; i < (BUF_SIZE-2); i++ )
    buf[i] = 'A' + (i % 26);
  buf[BUF_SIZE-2] = '\r';
  buf[BUF_SIZE-1] = '\n';

  Serial << "File size " << FILE_SIZE_MB << " MB" << endl
         << "Buffer size " << BUF_SIZE << " bytes" << endl
         << "Starting write test, please wait." << endl << endl;

  // do write test
  uint32_t n = FILE_SIZE/sizeof( buf );
  Serial << "write speed and latency" << endl
         << "speed,max,min,avg" << endl
         << "KB/Sec,usec,usec,usec" << endl;
  for( uint8_t nTest = 0; nTest < WRITE_COUNT; nTest ++ )
  {
    FatFs.remove( "bench.dat" );
    if( ! file.open( "bench.dat", FA_WRITE | FA_READ | FA_CREATE_ALWAYS ))
      error( "open failed" );
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    t = millis();
    for( uint32_t i = 0; i < n; i++ )
    {
      uint32_t m = micros();
      if( file.write( buf, sizeof( buf )) != sizeof( buf ))
        error( "write failed" );
      m = micros() - m;
      if( maxLatency < m )
        maxLatency = m;
      if( minLatency > m )
        minLatency = m;
      totalLatency += m;
    }
//    file.sync();
    t = millis() - t;
    s = file.fileSize();
    Serial << s/t <<',' << maxLatency << ',' << minLatency
           << ',' << totalLatency/n << endl;
  }

  Serial << endl << "Starting read test, please wait." << endl
         << endl <<F("read speed and latency") << endl
         << F("speed,max,min,avg") << endl
         << F("KB/Sec,usec,usec,usec") << endl;

  // do read test
  for( uint8_t nTest = 0; nTest < READ_COUNT; nTest++ )
  {
    file.seekSet( 0 );
    maxLatency = 0;
    minLatency = 9999999;
    totalLatency = 0;
    t = millis();
    for( uint32_t i = 0; i < n; i++ )
    {
      buf[BUF_SIZE-1] = 0;
      uint32_t m = micros();
      if( file.read( buf, sizeof(buf)) != sizeof(buf))
        error( "read failed" );
      m = micros() - m;
      if( maxLatency < m )
        maxLatency = m;
      if( minLatency > m )
        minLatency = m;
      totalLatency += m;
      if( buf[BUF_SIZE-1] != '\n' )
        error("data check");
    }
    t = millis() - t;
    Serial << s/t <<',' << maxLatency << ',' << minLatency
           << ',' << totalLatency/n << endl;
  }
  Serial << endl << "Done" << endl;
  file.close();
}

//------------------------------------------------------------------------------
void printError( int ok, char * msg )
{
  if( ok )
    return;
  Serial << msg << ": " << FatFs.error() << endl;
  while( 1 ) ;
}

//------------------------------------------------------------------------------
void error( char* s )
{
  Serial << s << endl;
  while( 1 ) ;
}

//------------------------------------------------------------------------------
#include <malloc.h>

extern char _end;
extern "C" char *sbrk(int i);

void freeRam()
{
  char *ramstart = (char *) 0x20070000;
  char *ramend = (char *) 0x20088000;
  char *heapend = sbrk(0);
  register char * stack_ptr asm( "sp" );
  struct mallinfo mi = mallinfo();
  Serial << "Ram utilisee (octets): " << endl
    << "  dynamique: " << mi.uordblks << endl
    << "  statique:  " << &_end - ramstart << endl
    << "  pile:      " << ramend - stack_ptr << endl; 
  Serial << "Estimation Ram libre: " << stack_ptr - heapend + mi.fordblks << endl << endl;
}

