/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for AT49xV162AT(16-bit Bus)        */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AT49xV162AT Flash",        // Device Name
   EXT16BIT,                   // Device Type
   0x000000,                   // Device Start Address
   0x200000,                   // Device Size in Bytes (2MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
   0x10000, 0x000000,          // Sector Size 64kB (31 Sectors)
   0x02000, 0x1F0000,          // Sector Size  8kB (8 Sectors)
   SECTOR_END
};
