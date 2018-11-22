/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for MAC71xx Data Flash                    */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures


struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "MAC71xx Data Flash",                 // Device Name
  ONCHIP,                               // Device Type
  0x00000000,                           // Device Start Address
  0x00008000,                           // Device Size in Bytes (32 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  100,                                  // Program Page Timeout 100 mSec
  3000,                                 // Erase Sector Timeout 3000 mSec

  // Specify Size and Address of Sectors
  0x00400, 0x000000,                    // Sector Size  1kB (32  Sectors)
  SECTOR_END
};
