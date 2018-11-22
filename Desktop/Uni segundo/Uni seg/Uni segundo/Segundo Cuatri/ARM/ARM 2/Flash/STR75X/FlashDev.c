/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for ST Microelectronics STR75X     */
/*                                      Flash Bank 0 & Bank 1          */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


#ifdef FLASH_BANK0
#ifdef BS256
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR75X Bank 0 256kB Flash",// Device Name 
   ONCHIP,                     // Device Type
   0x20000000,                 // Device Start Address
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
#ifdef BS128
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR75X Bank 0 128kB Flash",// Device Name 
   ONCHIP,                     // Device Type
   0x20000000,                 // Device Start Address
   0x00020000,                 // Device Size in Bytes (128kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   5000,                       // Erase Sector Timeout 5000 mSec

// Specify Size and Address of Sectors
   0x002000, 0x000000,         // Sector Size  8kB (4 Sectors)
   0x008000, 0x008000,         // Sector Size 32kB (1 Sector)
   0x010000, 0x010000,         // Sector Size 64kB (2 Sectors) 
   SECTOR_END
};
#endif
#ifdef BS64
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR75X Bank 0 64kB Flash", // Device Name 
   ONCHIP,                     // Device Type
   0x20000000,                 // Device Start Address
   0x00010000,                 // Device Size in Bytes (64kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   5000,                       // Erase Sector Timeout 5000 mSec

// Specify Size and Address of Sectors
   0x002000, 0x000000,         // Sector Size  8kB (4 Sectors)
   0x008000, 0x008000,         // Sector Size 32kB (1 Sector)
   SECTOR_END
};
#endif
#endif

#ifdef FLASH_BANK1
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR75X Bank 1 Flash",      // Device Name 
   ONCHIP,                     // Device Type
   0x200C0000,                 // Device Start Address
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
