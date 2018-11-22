/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2005 - 2007                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for flash algorithm using External SPI    */
/*               Flash ST M25P40 programming with AustriaMicroSystems AS352x  */
/*               Devices                                                      */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


                               // Page Size 1056 Bytes
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AS352x ST M25P40 SPI Flash",  // Device Name
   EXTSPI,                     // Device Type
   0x30000000,                 // Device Start Address
   0x00080000,                 // Device Size in Bytes (512kB)
   256,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3 Sec

// Specify Size and Address of Sectors
   0x10000, 0x000000,          // Sector Size 64 kB
   SECTOR_END
};
