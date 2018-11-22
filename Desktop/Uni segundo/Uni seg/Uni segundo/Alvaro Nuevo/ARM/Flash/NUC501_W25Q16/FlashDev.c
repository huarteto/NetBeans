/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2006-2010 Keil - An ARM Company.                     */
/*  All rights reserved.                                               */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.c:  Device Description for External SPI Flash             */
/*               W25Q16 programming with Nuvoton NUC501 Devices        */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"         // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,              // Driver Version, do not modify!
   "NUC501_W25Q16 SPI Flash",   // Device Name
   EXTSPI,                      // Device Type
   0x40000000,                  // Device Start Address
   0x200000,                    // Device Size in Bytes (2MB)
   256,                         // Programming Page Size
   0,                           // Reserved, must be 0
   0xFF,                        // Initial Content of Erased Memory
   100,                         // Program Page Timeout 100 mSec
   3000,                        // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x01000, 0x000000,           // Sector Size  4kB (812 Sectors)
   SECTOR_END
};
