/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for External SPI Flash                    */
/*               M25P64 programming with ST STM32F10x Devices                 */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STM32F10x M25P64 SPI Flash",//Device Name
   EXTSPI,                     // Device Type
   0xC0000000,                 // Device Start Address
   0x00800000,                 // Device Size in Bytes (8MB)
   256,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x10000, 0x000000,          // Sector Size 65kB (128 Sectors)
   SECTOR_END
};
