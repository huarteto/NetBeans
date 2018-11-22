/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for AT91SAM9XE internal Embedded Flash    */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures


#ifdef SIZE_128k
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,             // Driver Version, do not modify!
  "AT91SAM9 128kB Flash",     // Device Name
  ONCHIP,                     // Device Type
  0x00200000,                 // Device Start Address
  0x00020000,                 // Device Size in Bytes (128kB)
  512,                        // Programming Page Size
  0,                          // Reserved, must be 0
  0xFF,                       // Initial Content of Erased Memory
  100,                        // Program Page Timeout 100 mSec
  1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
  0x0200, 0x000000,           // Sector Size 512B (256 Sectors)
  SECTOR_END
};
#endif

#ifdef SIZE_256k
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,             // Driver Version, do not modify!
  "AT91SAM9 256kB Flash",     // Device Name
  ONCHIP,                     // Device Type
  0x00200000,                 // Device Start Address
  0x00040000,                 // Device Size in Bytes (256kB)
  512,                        // Programming Page Size
  0,                          // Reserved, must be 0
  0xFF,                       // Initial Content of Erased Memory
  100,                        // Program Page Timeout 100 mSec
  1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
  0x0200, 0x000000,           // Sector Size 512B (512 Sectors)
  SECTOR_END
};
#endif

#ifdef SIZE_512k
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,             // Driver Version, do not modify!
  "AT91SAM9 512kB Flash",     // Device Name
  ONCHIP,                     // Device Type
  0x00200000,                 // Device Start Address
  0x00080000,                 // Device Size in Bytes (512kB)
  512,                        // Programming Page Size
  0,                          // Reserved, must be 0
  0xFF,                       // Initial Content of Erased Memory
  100,                        // Program Page Timeout 100 mSec
  1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
  0x0200, 0x000000,           // Sector Size 512B (1024 Sectors)
  SECTOR_END
};
#endif

