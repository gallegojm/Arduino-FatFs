/*
 * This file is part of a class to wrap FatFs library from ChaN
 * Copyright (c) 2015 by Jean-Michel Gallego
 *
 * This file includes parts of file SdSpiCard.h from library SdFat
 *   developped by William Greiman
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

#ifndef FatFsCard_h
#define FatFsCard_h

//  FatFsCard class for V2 SD/SDHC cards

#include <Arduino.h>
#include <SPI.h>
// #include <SdFatConfig.h>
#include <FatFsInfo.h>

//==============================================================================
/**
 * \class FatFsCard
 * \brief Raw access to SD and SDHC flash memory cards via SPI protocol.
 */

 class FatFsCard
{
 public:
  /** Construct an instance of FatFsCard. */
  FatFsCard() : m_errorCode(SD_CARD_ERROR_INIT_NOT_CALLED), m_type(0) {}

  /** Initialize the SD card.
   * \param[in] spi SPI object.
   * \param[in] chipSelectPin SD chip select pin.
   * \param[in] sckDivisor SPI clock divisor.
   * \return true for success else false.
   */
  bool begin( uint8_t chipSelectPin, uint8_t sckDivisor );

  /**
   *  Set SD error code.
   *  \param[in] code value for error code.
   */

   void error(uint8_t code)
  {
    m_errorCode = code;
  }

  /**
   * \return code for the last error. See FatFsCard.h for a list of error codes.
   */
  int errorCode() const
  {
    return m_errorCode;
  }

  /** \return error data for last error. */
  int errorData() const
  {
    return m_status;
  }

  /**
   * Check for busy.  MISO low indicates the card is busy.
   *
   * \return true if busy else false.
   */
  bool isBusy();

  /**
   * Read a 512 byte block from an SD card.
   *
   * \param[in] block Logical block to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readBlock( uint32_t block, uint8_t* dst );

  /**
   * Read multiple 512 byte blocks from an SD card.
   *
   * \param[in] block Logical block to be read.
   * \param[in] count Number of blocks to be read.
   * \param[out] dst Pointer to the location that will receive the data.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readBlocks( uint32_t block, uint8_t* dst, size_t count );

  /** Read one data block in a multiple block read sequence
   *
   * \param[out] dst Pointer to the location for the data to be read.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readData( uint8_t *dst );
  
  /** Start a read multiple blocks sequence.
   *
   * \param[in] blockNumber Address of first block in sequence.
   *
   * \note This function is used with readData() and readStop() for optimized
   * multiple block reads.  SPI chipSelect must be low for the entire sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readStart( uint32_t blockNumber );
  
  /** End a read multiple blocks sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool readStop();
  
  /** Return the card type: SD V1, SD V2 or SDHC
   * \return 0 - SD V1, 1 - SD V2, or 3 - SDHC.
   */
  int type() const
  {
    return m_type;
  }

  /**
   * Writes a 512 byte block to an SD card.
   *
   * \param[in] blockNumber Logical block to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeBlock( uint32_t blockNumber, const uint8_t* src );
  
  /**
   * Write multiple 512 byte blocks to an SD card.
   *
   * \param[in] block Logical block to be written.
   * \param[in] count Number of blocks to be written.
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeBlocks( uint32_t block, const uint8_t* src, size_t count );
  
  /** Write one data block in a multiple block write sequence
   * \param[in] src Pointer to the location of the data to be written.
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeData( const uint8_t* src );
  
  /** Start a write multiple blocks sequence.
   *
   * \param[in] blockNumber Address of first block in sequence.
   * \param[in] eraseCount The number of blocks to be pre-erased.
   *
   * \note This function is used with writeData() and writeStop()
   * for optimized multiple block writes.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeStart( uint32_t blockNumber, uint32_t eraseCount );
  
  /** End a write multiple blocks sequence.
   *
   * \return The value true is returned for success and
   * the value false is returned for failure.
   */
  bool writeStop();
  bool waitNotBusy( uint16_t timeoutMillis );

 private:
  // private functions
  uint8_t cardAcmd( uint8_t cmd, uint32_t arg ) {
    cardCommand(CMD55, 0);
    return cardCommand(cmd, arg);
  }
  uint8_t cardCommand( uint8_t cmd, uint32_t arg );
  bool readData( uint8_t* dst, size_t count );
  void chipDeselect();
  void chipSelect();
  void type (uint8_t value) { m_type = value; }
  bool writeData( uint8_t token, const uint8_t* src );

  uint32_t spiFrec( uint8_t spiDivisor );
  uint8_t spiReceive();
  uint8_t spiReceive( uint8_t* buf, size_t n );
  void spiSend( uint8_t data );
  void spiSend( const uint8_t* buf, size_t n );

  uint8_t m_CSPin;
  uint8_t m_errorCode;
  uint32_t m_spifrec;
  uint8_t m_status;
  uint8_t m_type;
};
#endif  // FatFsCard_h
