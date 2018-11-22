/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2010                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Device Description for Spansion S29GLxxxA/N/P (16-bit bus)   */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#ifdef S29GL016
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29GL016 Flash",           // Device Name 
   EXT16BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x00200000,                 // Device Size in Bytes (2MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   150,                        // Program Page Timeout  150 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
// Union of top and bottom boot sector
   0x02000,       0x00000,     // Sector Size 8kB  (  8 Sectors)
   0x10000,   1 * 0x10000,     // Sector Size 64kB ( 30 Sectors)
   0x02000,  31 * 0x10000,     // Sector Size 8kB  (  8 Sectors)
   SECTOR_END
};
#endif

#ifdef S29GL032
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29GL032 Flash",           // Device Name 
   EXT16BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x00400000,                 // Device Size in Bytes (4MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   150,                        // Program Page Timeout  150 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
// Union of top and bottom boot sector
   0x02000,       0x00000,     // Sector Size 8kB  (  8 Sectors)
   0x10000,   1 * 0x10000,     // Sector Size 64kB ( 62 Sectors)
   0x02000,  63 * 0x10000,     // Sector Size 8kB  (  8 Sectors)
   SECTOR_END
};
#endif

#ifdef S29GL064
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29GL064 Flash",           // Device Name 
   EXT16BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x00800000,                 // Device Size in Bytes (8MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   150,                        // Program Page Timeout  150 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
// Union of top and bottom boot sector
   0x02000,       0x00000,     // Sector Size 8kB  (  8 Sectors)
   0x10000,   1 * 0x10000,     // Sector Size 64kB (126 Sectors)
   0x02000, 127 * 0x10000,     // Sector Size 8kB  (  8 Sectors)
   SECTOR_END
};
#endif

#ifdef S29GL128
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29GL128 Flash",           // Device Name 
   EXT16BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x01000000,                 // Device Size in Bytes (16MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   150,                        // Program Page Timeout  150 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
   0x20000,     0x00000,       // Sector Size 128kB (128 Sectors)
   SECTOR_END
};
#endif

#ifdef S29GL256
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29GL256 Flash",           // Device Name 
   EXT16BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x02000000,                 // Device Size in Bytes (32MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   150,                        // Program Page Timeout  150 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
   0x20000,     0x00000,       // Sector Size 128kB (256 Sectors)
   SECTOR_END
};
#endif

#ifdef S29GL512
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29GL512 Flash",           // Device Name 
   EXT16BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x04000000,                 // Device Size in Bytes (64MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   150,                        // Program Page Timeout  150 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
   0x20000,     0x00000,       // Sector Size 128kB (512 Sectors)
   SECTOR_END
};
#endif

#ifdef S29GL01G
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29GL01G Flash",           // Device Name 
   EXT16BIT,                   // Device Type
   0x00000000,                 // Device Start Address
   0x08000000,                 // Device Size in Bytes (128MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   150,                        // Program Page Timeout  150 mSec
   4000,                       // Erase Sector Timeout 4000 mSec

// Specify Size and Address of Sectors
   0x20000,     0x00000,       // Sector Size 128kB (1024 Sectors)
   SECTOR_END
};
#endif
