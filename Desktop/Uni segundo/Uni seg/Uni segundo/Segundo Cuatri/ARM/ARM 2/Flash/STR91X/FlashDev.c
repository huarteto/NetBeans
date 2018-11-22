/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for ST Microelectronics STR91x     */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#ifdef BANK0
#ifdef ICP
#ifdef STR91xFxx2
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx2 Flash Bank0 ICP",  // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00040000,                 // Device Size in Bytes (256kB)
   0x00010000,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   6000,                       // Program Page Timeout 6000 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x010000, 0x000000,         // Sector Size 64kB (4 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx4
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx4 Flash Bank0 ICP",  // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00080000,                 // Device Size in Bytes (512kB)
   0x00010000,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   6000,                       // Program Page Timeout 6000 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x010000, 0x000000,         // Sector Size 64kB (8 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx6
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx6 Flash Bank0 ICP",  // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00100000,                 // Device Size in Bytes (1MB)
   0x00010000,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   6000,                       // Program Page Timeout 6000 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x010000, 0x000000,         // Sector Size 64kB (16 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx7
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx7 Flash Bank0 ICP",  // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00200000,                 // Device Size in Bytes (2MB)
   0x00010000,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   6000,                       // Program Page Timeout 6000 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x010000, 0x000000,         // Sector Size 64kB (32 Sectors)
   SECTOR_END
};
#endif
#else
#ifdef STR91xFxx2
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx2 Flash Bank0",   // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00040000,                 // Device Size in Bytes (256kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x010000, 0x000000,         // Sector Size 64kB (4 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx4
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx4 Flash Bank0",   // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00080000,                 // Device Size in Bytes (512kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x010000, 0x000000,         // Sector Size 64kB (8 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx6
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx6 Flash Bank0",   // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00100000,                 // Device Size in Bytes (1MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x010000, 0x000000,         // Sector Size 64kB (16 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx7
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx7 Flash Bank0",   // Device Name
   ONCHIP,                     // Device Type
   0x00000000,                 // Device Start Address
   0x00200000,                 // Device Size in Bytes (2MB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x010000, 0x000000,         // Sector Size 64kB (32 Sectors)
   SECTOR_END
};
#endif
#endif
#endif

#ifdef BANK1
#ifdef ICP
#ifdef STR91xFxx2
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx2 Flash Bank1 ICP",  // Device Name
   ONCHIP,                     // Device Type
   0x00400000,                 // Device Start Address
   0x00008000,                 // Device Size in Bytes (32kB)
   0x00002000,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   6000,                       // Program Page Timeout 6000 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x002000, 0x000000,         // Sector Size 8kB (4 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx4
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx4 Flash Bank1 ICP",  // Device Name
   ONCHIP,                     // Device Type
   0x00400000,                 // Device Start Address
   0x00008000,                 // Device Size in Bytes (32kB)
   0x00002000,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   6000,                       // Program Page Timeout 6000 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x002000, 0x000000,         // Sector Size 8kB (4 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx6
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx6 Flash Bank1 ICP",  // Device Name
   ONCHIP,                     // Device Type
   0x00400000,                 // Device Start Address
   0x00020000,                 // Device Size in Bytes (128kB)
   0x00004000,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   6000,                       // Program Page Timeout 6000 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x004000, 0x000000,         // Sector Size 16kB (8 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx7
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx7 Flash Bank1 ICP",  // Device Name
   ONCHIP,                     // Device Type
   0x00400000,                 // Device Start Address
   0x00020000,                 // Device Size in Bytes (128kB)
   0x00004000,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   6000,                       // Program Page Timeout 6000 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
   0x004000, 0x000000,         // Sector Size 16kB (8 Sectors)
   SECTOR_END
};
#endif
#else
#ifdef STR91xFxx2
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx2 Flash Bank1",   // Device Name
   ONCHIP,                     // Device Type
   0x00400000,                 // Device Start Address
   0x00008000,                 // Device Size in Bytes (32kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
   0x002000, 0x000000,         // Sector Size 8kB (4 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx4
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx4 Flash Bank1",   // Device Name
   ONCHIP,                     // Device Type
   0x00400000,                 // Device Start Address
   0x00008000,                 // Device Size in Bytes (32kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
   0x002000, 0x000000,         // Sector Size 8kB (4 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx6
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx6 Flash Bank1",   // Device Name
   ONCHIP,                     // Device Type
   0x00400000,                 // Device Start Address
   0x00020000,                 // Device Size in Bytes (128kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
   0x004000, 0x000000,         // Sector Size 16kB (8 Sectors)
   SECTOR_END
};
#endif
#ifdef STR91xFxx7
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91xFxx7 Flash Bank1",   // Device Name
   ONCHIP,                     // Device Type
   0x00400000,                 // Device Start Address
   0x00020000,                 // Device Size in Bytes (128kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   1000,                       // Erase Sector Timeout 1000 mSec

// Specify Size and Address of Sectors
   0x004000, 0x000000,         // Sector Size 16kB (8 Sectors)
   SECTOR_END
};
#endif
#endif
#endif

#ifdef OTP
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91x OTP ICP",           // Device Name
   ONCHIP,                     // Device Type
   0x00500000,                 // Device Start Address
   0x00000020,                 // Device Size in Bytes (32B)
   0x00000020,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   500,                        // Erase Sector Timeout 500 mSec

// Specify Size and Address of Sectors
   0x000020, 0x000000,         // Sector Size 32B
   SECTOR_END
};
#endif

#ifdef USERCODE
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91x User Code ICP",     // Device Name
   ONCHIP,                     // Device Type
   0x00510000,                 // Device Start Address
   0x00000004,                 // Device Size in Bytes (4B)
   0x00000004,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   500,                        // Erase Sector Timeout 500 mSec

// Specify Size and Address of Sectors
   0x000004, 0x000000,         // Sector Size 4B
   SECTOR_END
};
#endif

#ifdef CFG_SECURITY
struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "STR91x CFG & Security ICP",// Device Name
   ONCHIP,                     // Device Type
   0x00520000,                 // Device Start Address
   0x00000010,                 // Device Size in Bytes (8B+1B+7B)
   0x00000008,                 // Programming Page Size
   1,                          // Reserved, must be 1
   0x00,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   500,                        // Erase Sector Timeout 500 mSec

// Specify Size and Address of Sectors
   0x000008, 0x000000,         // Sector Size 8B
   SECTOR_END
};
#endif
