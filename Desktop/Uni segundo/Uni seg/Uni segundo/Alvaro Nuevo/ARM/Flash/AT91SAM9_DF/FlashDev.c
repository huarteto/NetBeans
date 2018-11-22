/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2005 - 2010                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for flash algorithm using Atmel           */
/*               AT91SAM926x Device for Programming External DataFlash        */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures


#ifdef P528_CS0                // Page Size 528 Bytes, Chip Select 0
  #ifdef ATSAM9G45
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AT91SAM9G45 DataFlash Page528 CS0",    // Device Name
   EXTSPI,                     // Device Type
   0x70000000,                 // Device Start Address
   0x00840000,                 // Device Size in Bytes (8.25MB)
   528,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   1000,                       // Program Page Timeout 1000 mSec
   1000,                       // Erase Sector Timeout 1000 mSec
// Specify Size and Address of Sectors
   0x1080, 0x000000,           // Sector Size 8448 bytes (block = 8*page)
   SECTOR_END
};
  #else
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AT91SAM9 DataFlash Page528 CS0",   // Device Name
   EXTSPI,                     // Device Type
   0x20000000,                 // Device Start Address
   0x00420000,                 // Device Size in Bytes (4.125MB)
   528,                        // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   100,                        // Erase Sector Timeout 100 mSec

// Specify Size and Address of Sectors
   0x1080, 0x000000,           // Sector Size 4224 bytes (block = 8*page)
   SECTOR_END
};
  #endif
#endif




#ifdef P1056_CS0               // Page Size 1056 Bytes, Chip Select 0
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AT91SAM9 DataFlash Page1056 CS0",   // Device Name
   EXTSPI,                     // Device Type
   0x20000000,                 // Device Start Address
   0x00840000,                 // Device Size in Bytes (8.25MB)
   1056,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   100,                        // Erase Sector Timeout 100 mSec

// Specify Size and Address of Sectors
   0x2100, 0x000000,           // Sector Size 8448 bytes (block = 8*page)
   SECTOR_END
};
#endif


#ifdef P1056_CS1               // Page Size 1056 Bytes, Chip Select 1
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "AT91SAM9 DataFlash Page1056 CS1",   // Device Name
   EXTSPI,                     // Device Type
   0x20000000,                 // Device Start Address
   0x00840000,                 // Device Size in Bytes (8.25MB)
   1056,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   100,                        // Erase Sector Timeout 100 mSec

// Specify Size and Address of Sectors
   0x2100, 0x000000,           // Sector Size 8448 bytes (block = 8*page)
   SECTOR_END
};
#endif
