/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for SST39x160x (16-bit Bus)        */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "SST39x160x Flash",         // Device Name
   EXT16BIT,                   // Device Type
   0x000000,                   // Device Start Address
   0x200000,                   // Device Size in Bytes (2MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   1000,                       // Erase Sector Timeout 1 Sec

// Specify Size and Address of Sectors
   0x001000, 0x000000,         // Sector Size 4kB (512 Sectors)
   SECTOR_END
};
