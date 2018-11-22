/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2009                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for flash algorithm using NXP LPC313x,    */
/*               LPC314x and LPC315x Device Series and NAND Flash with        */ 
/*               Large Pages (eg. Micron MT29F2G08ABD)                        */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "LPC31xx NAND Flash LP",    // Device Name
   EXT8BIT,                    // Device Type
   0x30000000,                 // Device Start Address
   0x10000000,                 // Device Size in Bytes (256MB)
   2048,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   300,                        // Program Page Timeout 300  mSec
   1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
   0x20000, 0x000000,          // Sector (block) Size 128kB
   SECTOR_END
};
