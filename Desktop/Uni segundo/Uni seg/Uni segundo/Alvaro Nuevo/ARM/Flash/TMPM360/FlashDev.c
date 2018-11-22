/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2003-2009 Keil Software. All rights reserved.        */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for Toshiba TMPM360Fx Flash        */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


#ifdef TMPM360_2048
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "TMPM360Fx 2048kB Flash",   // Device Name 
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00200000,                 // Device Size (2048kB)
   512,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x008000, 0x000000,         // Sector Size  32kB ( 2 Sectors)
   0x010000, 0x010000,         // Sector Size  64kB ( 1 Sector) 
   0x020000, 0x020000,         // Sector Size 128kB (14 Sectors) 
   0x010000, 0x1E0000,         // Sector Size  64kB ( 1 Sector) 
   0x008000, 0x1F0000,         // Sector Size  32kB ( 2 Sectors)
   SECTOR_END
};
#endif
