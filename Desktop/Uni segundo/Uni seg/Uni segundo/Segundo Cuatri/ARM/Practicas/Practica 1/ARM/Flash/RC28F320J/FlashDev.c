/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for Intel RC28F320J (16-bit Bus)   */
/*                                      Intel  E28F320J (16-bit Bus)   */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "RC28F320J Flash",          // Device Name 
   EXT16BIT,                   // Device Type
   0x000000,                   // Device Start Address
   0x400000,                   // Device Size in Bytes (4MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x20000, 0x000000,          // Sector Size 128kB (32 Sectors)
   SECTOR_END
};
