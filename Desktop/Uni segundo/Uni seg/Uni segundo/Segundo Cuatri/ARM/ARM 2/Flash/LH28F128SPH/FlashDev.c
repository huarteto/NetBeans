/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for Sharp LH28F128SPH (16-bit bus) */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "LH28F128SPH Flash",        // Device Name 
   EXT16BIT,                   // Device Type
   0x0000000,                  // Device Start Address
   0x1000000,                  // Device Size in Bytes (16MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
   0x20000, 0x000000,          // Sector Size 128kB (128 Sectors)
   SECTOR_END
};
