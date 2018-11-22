/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for Dual Samsung K8P5615UQA               */
/*               NOR Flash Memory (32-bit Bus)                                */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "K8P5615UQA Dual Flash",    // Device Name 
   EXT32BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x04000000,                 // Device Size in Bytes (64MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x20000, 0x0000000,         // Sector Size 2*64kB (2*4 Sectors)
   0x80000, 0x0080000,         // Sector Size 2*256kB (2*126 Sectors)
   0x20000, 0x3F80000,         // Sector Size 2*64kB (2*4 Sectors)
   SECTOR_END
};
