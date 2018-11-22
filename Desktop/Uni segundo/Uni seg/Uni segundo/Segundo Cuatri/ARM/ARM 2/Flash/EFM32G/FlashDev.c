/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for Energy Micro EFM32G            */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        /* FlashOS Structures */


struct FlashDevice const FlashDevice =
{
  FLASH_DRV_VERS, /* Driver Version, do not modify!   */
  "EFM32G",       /* Device Name                      */
  ONCHIP,         /* Device Type                      */
  0x00000000,     /* Device Start Address             */
  0x00020000,     /* Device Size without Boot Loader  */
  512,            /* Programming Page Size            */
  0,              /* Reserved, must be 0              */
  0xFF,           /* Initial Content of Erased Memory */
  100,            /* Program Page Timeout 100 mSec    */
  3000,           /* Erase Sector Timeout 3000 mSec   */

/* Specify Size and Address of Sectors */
  0x000200, 0x000000,          /* Sector Size  512B */
  SECTOR_END
};
