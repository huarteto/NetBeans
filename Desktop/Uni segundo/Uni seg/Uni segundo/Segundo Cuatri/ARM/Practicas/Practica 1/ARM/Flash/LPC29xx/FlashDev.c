/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for LPC29xx Device Series Flash           */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures


#ifdef SIZE_512kB
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "LPC29xx 512kB Flash",                // Device Name
  ONCHIP,                               // Device Type
  0x20000000,                           // Device Start Address
  0x80000,                              // Device Size in Bytes (512 kB)
  512,                                  // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x2000,  0x0000,                      // Sector Size   8kB ( 8 Sectors)
  0x10000, 0x10000,                     // Sector Size  64kB ( 7 Sectors)
  SECTOR_END
};
#endif

#ifdef SIZE_768kB
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "LPC29xx 768kB Flash",                // Device Name
  ONCHIP,                               // Device Type
  0x20000000,                           // Device Start Address
  0xC0000,                              // Device Size in Bytes (768 kB)
  512,                                  // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x2000,  0x0000,                      // Sector Size   8kB ( 8 Sectors)
  0x10000, 0x10000,                     // Sector Size  64kB (11 Sectors)
  SECTOR_END
};
#endif
