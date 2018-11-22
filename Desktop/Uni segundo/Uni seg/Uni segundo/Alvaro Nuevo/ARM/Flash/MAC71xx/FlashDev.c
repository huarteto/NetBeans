/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for MAC71xx Program Flash                 */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures


#ifdef MAC71x1
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "MAC71x1 Program Flash",              // Device Name
  ONCHIP,                               // Device Type
  0x000000,                             // Device Start Address
  0x80000,                              // Device Size in Bytes (512 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  100,                                  // Program Page Timeout 100 mSec
  3000,                                 // Erase Sector Timeout 3000 mSec

  // Specify Size and Address of Sectors
  0x01000, 0x000000,                    // Sector Size  4kB (128 Sectors)
  SECTOR_END
};
#endif

#ifdef MAC71x2
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "MAC71x2 Program Flash",              // Device Name
  ONCHIP,                               // Device Type
  0x000000,                             // Device Start Address
  0x40000,                              // Device Size in Bytes (256 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  100,                                  // Program Page Timeout 100 mSec
  3000,                                 // Erase Sector Timeout 3000 mSec

  // Specify Size and Address of Sectors
  0x01000, 0x000000,                    // Sector Size  4kB (64 Sectors)
  SECTOR_END
};
#endif

#ifdef MAC71x4
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "MAC71x4 Program Flash",              // Device Name
  ONCHIP,                               // Device Type
  0x000000,                             // Device Start Address
  0x60000,                              // Device Size in Bytes (384 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  100,                                  // Program Page Timeout 100 mSec
  3000,                                 // Erase Sector Timeout 3000 mSec

  // Specify Size and Address of Sectors
  0x01000, 0x000000,                    // Sector Size  4kB (96 Sectors)
  SECTOR_END
};
#endif

#ifdef MAC71x5
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "MAC71x5 Program Flash",              // Device Name
  ONCHIP,                               // Device Type
  0x000000,                             // Device Start Address
  0xC0000,                              // Device Size in Bytes (768 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  100,                                  // Program Page Timeout 100 mSec
  3000,                                 // Erase Sector Timeout 3000 mSec

  // Specify Size and Address of Sectors
  0x01000, 0x000000,                    // Sector Size  4kB (192 Sectors)
  SECTOR_END
};
#endif

#ifdef MAC71x6
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "MAC71x6 Program Flash",              // Device Name
  ONCHIP,                               // Device Type
  0x000000,                             // Device Start Address
  0x100000,                             // Device Size in Bytes (1 MB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  100,                                  // Program Page Timeout 100 mSec
  3000,                                 // Erase Sector Timeout 3000 mSec

  // Specify Size and Address of Sectors
  0x01000, 0x000000,                    // Sector Size  4kB (256 Sectors)
  SECTOR_END
};
#endif
