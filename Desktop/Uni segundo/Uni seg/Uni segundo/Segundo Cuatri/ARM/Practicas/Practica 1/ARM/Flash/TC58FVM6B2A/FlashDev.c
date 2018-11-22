/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for TC58FVM6B2A (16-bit Bus)       */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "TC58FVM6B2A Flash",
   EXT16BIT,                   // Device Type
   0x000000,                   // Device Start Address
   0x800000,                   // Device Size in Bytes (4MB)
   512,                        // Programming Page Size (Bytes)
   0,                          // Reserved, must be 0
   0x00,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   7000,                       // Erase Sector Timeout 7000 mSec

// Specify Size and Address of Sectors
   0x02000, 0x00000,           // Sector Size   8kB (  8 Sectors)
   0x10000, 0x10000,           // Sector Size  64kB (127 Sectors)
   SECTOR_END
};
