/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for AM29x128 (16-bit Bus)          */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#ifdef S29JL032H_TOP	 // S29JL032H Top Boot Device      Model x1(01/21/31/41)
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29JL032H_TOP Flash",      // Device Name 
   EXT16BIT,                   // Device Type
   0x0000000,                  // Device Start Address
   0x400000,                  // Device Size in Bytes (4MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   900,                        // Erase Sector Timeout 900 mSec
// Specify Size and Address of Sectors
   0x10000,     0x00000,       // Sector Size 64kB (63 Sectors)
   0x02000,     0x3F0000,      // Sector Size  8kB (8 Sectors)
   SECTOR_END
};
#endif

#ifdef S29JL032H_BOTTOM	 // S29JL032H Bottom Boot Device		Model x2(02/22/32/42)
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "S29JL032H_BOT Flash",       // Device Name 
   EXT16BIT,                   // Device Type
   0x0000000,                  // Device Start Address
   0x400000,                  // Device Size in Bytes (4MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   900,                        // Erase Sector Timeout 900 mSec
// Specify Size and Address of Sectors
   0x02000,     0x00000,       // Sector Size  8kB (8 Sectors)
   0x10000,     0x10000,       // Sector Size 64kB (63 Sectors)
   SECTOR_END
};
#endif

