/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for Dual Spansion S29x128 (32-bit bus)    */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29x128 Dual Flash",       // Device Name 
   EXT32BIT,                   // Device Type
   0x0000000,                  // Device Start Address
   0x2000000,                  // Device Size in Bytes (32MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
   0x10000,     0x00000,       // Sector Size 2*32kB  (  4 Sectors)
   0x40000, 4  *0x10000,       // Sector Size 2*128kB (126 Sectors)
   0x10000, 127*0x40000,       // Sector Size 2*32kB  (  4 Sectors)
   SECTOR_END
};
