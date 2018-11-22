/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2009                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for                                       */
/*               Dual Mxic MX29LV320B NOR Flash Memory (16-bit Bus)           */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "MX29LV320B_x2 Dual Flash", // Device Name 
   EXT16BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x00800000,                 // Device Size in Bytes (8MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
// Size   , Address
   0x04000, 0x0000000,         // Sector Size 2* 8kB (2* 8 Sectors)
   0x20000, 0x0020000,         // Sector Size 2*64kB (2*63 Sectors)
   SECTOR_END
};
