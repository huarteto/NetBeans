/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for flash algorithm using Freescale       */
/*               i.MX27 and NAND Flash with Small Pages                       */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "iMX27 NAND Flash SP",      // Device Name
   EXT8BIT,                    // Device Type
   0x00000000,                 // Device Start Address
   0x08000000,                 // Device Size in Bytes (128MB)
   512,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   200,                        // Program Page Timeout 200 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x4000, 0x000000,           // Sector Size 16kB
   SECTOR_END
};
