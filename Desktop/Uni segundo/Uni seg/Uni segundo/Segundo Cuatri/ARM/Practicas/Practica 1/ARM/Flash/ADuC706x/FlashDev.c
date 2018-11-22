/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2004 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for ADuC706X (32KB)                */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "ADuC706X Flash 32KB (v1.0)",// Device Name + version number
   ONCHIP,                     // Device Type
   0x00080000,                 // Device Start Address
   0x00007800,                 // Device Size in Bytes (30kB)
   512,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   500,                        // Program Page Timeout 500 mSec
   500,                        // Erase Sector Timeout 500 mSec

// Specify Size and Address of Sectors
   0x0200, 0x0000,             // Sector Size 512B
   SECTOR_END
};
