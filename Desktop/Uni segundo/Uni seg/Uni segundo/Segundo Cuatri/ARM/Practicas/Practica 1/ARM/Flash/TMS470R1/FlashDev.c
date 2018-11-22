/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for TMS470R1 Device Series Flash          */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures


#ifdef A64
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1A64 Flash",                  // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0x10000,                              // Device Size in Bytes (64 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x2000, 0x0000,                       // Sector Size   8kB ( 2 Sectors)
  0x4000, 0x4000,                       // Sector Size  16kB ( 3 Sectors)
  SECTOR_END
};
#endif

#ifdef A128
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1A128 Flash",                 // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0x20000,                              // Device Size in Bytes (128 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x2000, 0x00000,                      // Sector Size   8kB ( 2 Sectors)
  0x4000, 0x04000,                      // Sector Size  16kB ( 6 Sectors)
  0x2000, 0x1C000,                      // Sector Size   8kB ( 2 Sectors)
  SECTOR_END
};
#endif

#ifdef A256
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1A256 Flash",                 // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0x40000,                              // Device Size in Bytes (256 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x2000, 0x00000,                      // Sector Size   8kB ( 4 Sectors)
  0x8000, 0x08000,                      // Sector Size  32kB ( 6 Sectors)
  0x2000, 0x38000,                      // Sector Size   8kB ( 4 Sectors)
  SECTOR_END
};
#endif

#ifdef A288_BANK0
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1A288 Flash (Bank 0)",        // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0x8000,                               // Device Size in Bytes (32 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x2000, 0x00000,                      // Sector Size   8kB ( 4 Sectors)
  SECTOR_END
};
#endif

#ifdef A288_BANK1
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1A288 Flash (Bank 1)",        // Device Name
  ONCHIP,                               // Device Type
  0x40000,                              // Device Start Address
  0x40000,                              // Device Size in Bytes (256 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x10000, 0x40000,                     // Sector Size  64kB ( 4 Sectors)
  SECTOR_END
};
#endif

#ifdef A384
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1A384 Flash",                 // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0x60000,                              // Device Size in Bytes (384 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x2000, 0x00000,                      // Sector Size   8kB ( 2 Sectors)
  0x4000, 0x04000,                      // Sector Size  16kB ( 6 Sectors)
  0x2000, 0x1C000,                      // Sector Size   8kB ( 2 Sectors)
  0x8000, 0x20000,                      // Sector Size  32kB ( 4 Sectors)
  0x8000, 0x40000,                      // Sector Size  32kB ( 4 Sectors)
  SECTOR_END
};
#endif

#ifdef B512
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1B512 Flash",                 // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0x80000,                              // Device Size in Bytes (512 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x04000, 0x000000,                    // Sector Size  16kB ( 2 Sectors)
  0x08000, 0x008000,                    // Sector Size  32kB ( 6 Sectors)
  0x04000, 0x038000,                    // Sector Size  16kB ( 2 Sectors)
  0x10000, 0x040000,                    // Sector Size  64kB ( 4 Sectors)
  SECTOR_END
};
#endif

#ifdef B768
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1B768 Flash",                 // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0xC0000,                              // Device Size in Bytes (768 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x04000, 0x000000,                    // Sector Size  16kB ( 2 Sectors)
  0x08000, 0x008000,                    // Sector Size  32kB ( 6 Sectors)
  0x04000, 0x038000,                    // Sector Size  16kB ( 2 Sectors)
  0x10000, 0x040000,                    // Sector Size  64kB ( 4 Sectors)
  0x10000, 0x080000,                    // Sector Size  64kB ( 4 Sectors)
  SECTOR_END
};
#endif

#ifdef B1M
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS470R1B1M Flash",                  // Device Name
  ONCHIP,                               // Device Type
  0x000000,                             // Device Start Address
  0x100000,                             // Device Size in Bytes (1 MB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x10000, 0x000000,                    // Sector Size  64kB (16 Sectors)
  SECTOR_END
};
#endif

