/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for flash algorithm using Samsung  */
/*               S3F4Axxx Device Series Flash                          */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


#ifdef SIZE_1024
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "S3F4Axxx 1024kB Flash",              // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0x100000,                             // Device Size in Bytes (1024 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x4000, 0x0000,                       // Sector Size  16kB (64 sectors)
  SECTOR_END
};
#endif

#ifdef SIZE_512
struct FlashDevice const FlashDevice  =  {
  FLASH_DRV_VERS,                       // Driver Version, do not modify!
  "S3F4Axxx 512kB Flash",               // Device Name
  ONCHIP,                               // Device Type
  0x0,                                  // Device Start Address
  0x80000,                              // Device Size in Bytes (512 kB)
  1024,                                 // Programming Page Size
  0,                                    // Reserved, must be 0
  0xFF,                                 // Initial Content of Erased Memory
  500,                                  // Program Page Timeout 500  mSec
  5000,                                 // Erase Sector Timeout 5000 mSec

  // Specify Size and Address of Sectors
  0x4000, 0x0000,                       // Sector Size  16kB (32 sectors)
  SECTOR_END
};
#endif
