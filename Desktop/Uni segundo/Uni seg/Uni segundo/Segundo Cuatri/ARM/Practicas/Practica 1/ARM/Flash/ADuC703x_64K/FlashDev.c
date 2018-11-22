/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2004 - 2009                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for ADuC703X (64KB)                */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "ADuC703X Flash 64KB (v1.1)",// Device Name + version number
   ONCHIP,                     // Device Type
   0x00080000,                 // Device Start Address
   0x0000F800,                 // Device Size in Bytes (62kB)
   512,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   500,                        // Program Page Timeout 500 mSec
   500,                        // Erase Sector Timeout 500 mSec

// Specify Size and Address of Sectors
   0x0200, 0x0000,             // Sector Size 512B
   SECTOR_END
};
