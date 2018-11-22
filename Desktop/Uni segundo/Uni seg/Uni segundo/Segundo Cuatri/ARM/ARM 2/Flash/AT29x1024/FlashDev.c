/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for AT29x1024 (16-bit Bus)         */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AT29x1024 Flash",          // Device Name
   EXT16BIT,                   // Device Type
   0x000000,                   // Device Start Address
   0x20000,                    // Device Size in Bytes (128kB)
   256,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   100,                        // Erase Sector Timeout 100 mSec

// Specify Size and Address of Sectors
   0x00100, 0x00000,           // Sector Size 256B (512 Sectors)
   SECTOR_END
};
