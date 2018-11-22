/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2010                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for TMS570LS Device Series Flash          */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures


#ifdef  TMS570LS10XX

struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS570LS 1M Flash",                  // Device Name
  ONCHIP,                               // Device Type
  0x000000,                             // Device Start Address
  0x100000,                             // Device Size in Bytes (1 MB)
  32768,                                // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  4000,                                 // Program Page Timeout 4000 mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x8000, 0x000000,                     // Sector Size  32kB ( 3 Sectors)
  0x2000, 0x018000,                     // Sector Size   8kB ( 2 Sectors)
  0x4000, 0x01C000,                     // Sector Size  16kB ( 1 Sector)
  0x10000,0x020000,                     // Sector Size  64kB ( 2 Sectors)
  0x20000,0x040000,                     // Sector Size 128kB ( 6 Sectors)
  SECTOR_END
};
#endif

#ifdef  TMS570LS20XX

struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS570LS 2M Flash",                  // Device Name
  ONCHIP,                               // Device Type
  0x000000,                             // Device Start Address
  0x200000,                             // Device Size in Bytes (2 MB)
  32768,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  4000,                                 // Program Page Timeout 4000 mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x8000, 0x000000,                     // Sector Size  32kB ( 3 Sectors)
  0x2000, 0x018000,                     // Sector Size   8kB ( 2 Sectors)
  0x4000, 0x01C000,                     // Sector Size  16kB ( 1 Sector)
  0x10000,0x020000,                     // Sector Size  64kB ( 2 Sectors)
  0x20000,0x040000,                     // Sector Size 128kB (14 Sectors)

  SECTOR_END
};
#endif

#ifdef  TMS570LS20XXECC

struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS570LS 2M Flash ECC",              // Device Name
  ONCHIP,                               // Device Type
  0x400000,                             // Device Start Address
  0x100000,                             // Device Size in Bytes (1 MB)
  32768,                                // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  4000,                                 // Program Page Timeout 4000 mSec
  5000,                                 // Erase Sector Timeout 5000 mSec
  // Specify Size and Address of Sectors
  // Specify Size and Address of Sectors
  0x8000, 0x000000,                     // Sector Size  32kB ( 3 Sectors)
  0x2000, 0x018000,                     // Sector Size   8kB ( 2 Sectors)
  0x4000, 0x01C000,                     // Sector Size  16kB ( 1 Sector)
  0x10000,0x020000,                     // Sector Size  64kB ( 2 Sectors)
  0x20000,0x040000,                     // Sector Size 128kB ( 6 Sectors)

  SECTOR_END
};
#endif

#ifdef  TMS570LS10XXECC

struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "TMS570LS 1M Flash ECC",              // Device Name
  ONCHIP,                               // Device Type
  0x400000,                             // Device Start Address
  0x080000,                             // Device Size in Bytes (1 MB)
  32768,                                // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  4000,                                 // Program Page Timeout 4000 mSec
  5000,                                 // Erase Sector Timeout 5000 mSec
  // Specify Size and Address of Sectors
  // Specify Size and Address of Sectors
  0x8000, 0x000000,                     // Sector Size  32kB ( 3 Sectors)
  0x2000, 0x018000,                     // Sector Size   8kB ( 2 Sectors)
  0x4000, 0x01C000,                     // Sector Size  16kB ( 1 Sector)
  0x10000,0x020000,                     // Sector Size  64kB ( 2 Sectors)
  0x20000,0x040000,                     // Sector Size 128kB ( 2 Sectors)

  SECTOR_END
};
#endif
