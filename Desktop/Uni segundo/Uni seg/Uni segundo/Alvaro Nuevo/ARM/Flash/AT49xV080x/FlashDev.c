/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for AT49xV080x (8-bit Bus)         */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AT49xV080x Flash",         // Device Name
   EXT8BIT,                    // Device Type
   0x000000,                   // Device Start Address
   0x100000,                   // Device Size in Bytes (1MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   10000,                      // Erase Sector Timeout 10 Sec

// Specify Size and Address of Sectors
   0x100000, 0x000000,         // Sector Size 1MB (1 Single Sector)
   SECTOR_END
};
