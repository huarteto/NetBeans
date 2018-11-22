/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2010 Keil Software. All rights reserved.             */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for Toshiba TMPM380 Flash          */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


#ifdef TMPM38x_64
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "TMPM38x 64kB Flash",       // Device Name 
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00010000,                 // Device Size (64kB)
   256,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x004000, 0x000000,         // Sector Size  16kB ( 2 Sectors)
   0x008000, 0x008000,         // Sector Size  32kB ( 1 Sector) 
   SECTOR_END
};
#endif


#ifdef TMPM38x_128
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "TMPM38x 128kB Flash",      // Device Name 
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00020000,                 // Device Size (128kB)
   256,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x004000, 0x000000,         // Sector Size  16kB ( 2 Sectors)
   0x008000, 0x008000,         // Sector Size  32kB ( 1 Sector) 
   0x010000, 0x010000,         // Sector Size  64kB ( 1 Sectors) 
   SECTOR_END
};
#endif


#ifdef TMPM38x_256
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "TMPM38x 256kB Flash",      // Device Name 
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00040000,                 // Device Size (256kB)
   256,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x004000, 0x000000,         // Sector Size  16kB ( 2 Sectors)
   0x008000, 0x008000,         // Sector Size  32kB ( 1 Sector) 
   0x010000, 0x010000,         // Sector Size  64kB ( 3 Sectors) 
   SECTOR_END
};
#endif

