/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for Intel 				           */
/*               TE/JS/PC/RC28F640P30B (16-bit Bus)                    */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "RC28F640P30B Flash",       // Device Name 
   EXT16BIT,                   // Device Type
   0x000000,                   // Device Start Address
   0x800000,                   // Device Size in Bytes (8MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
   0x08000, 0x000000,          // Sector Size 32kB  ( 4 Sectors)
   0x20000, 0x020000,          // Sector Size 128kB (63 Sectors)
   SECTOR_END
};
