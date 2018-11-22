/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for ST Microelectronics STR71X     */
/*                                      Flash Bank 0 & Bank 1          */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


#ifdef FLASH_BANK0
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR71X Bank 0 Flash",      // Device Name 
   ONCHIP,                     // Device Type
   0x40000000,                 // Device Start Address
   0x00040000,                 // Device Size in Bytes (256kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   5000,                       // Erase Sector Timeout 5000 mSec

// Specify Size and Address of Sectors
   0x002000, 0x000000,         // Sector Size  8kB (4 Sectors)
   0x008000, 0x008000,         // Sector Size 32kB (1 Sector)
   0x010000, 0x010000,         // Sector Size 64kB (3 Sectors) 
   SECTOR_END
};
#endif

#ifdef FLASH_BANK1
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR71X Bank 1 Flash",      // Device Name 
   ONCHIP,                     // Device Type
   0x400C0000,                 // Device Start Address
   0x00004000,                 // Device Size in Bytes (16kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   2000,                       // Erase Sector Timeout 2000 mSec

// Specify Size and Address of Sectors
   0x002000, 0x000000,         // Sector Size  8kB (2 Sectors)
   SECTOR_END
};
#endif
