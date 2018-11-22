/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Flash Programming Functions adapted for                      */
/*               Intel TE/JS/PC/RC28F256P30B (16-bit Bus)                     */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

#define STACK_SIZE   64                 // Stack Size


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
  return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  return (0);
}


/*
 *  Lock Sector
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int LockSector (unsigned long adr) {
  unsigned char ls;

  M16(adr) = 0x60;                      // Start Lock Operation
  M16(adr) = 0x01;                      // Lock Sector
  M16(adr) = 0x90;                      // Read Electronic Signature
  ls = M16(adr | (0x02 << 1));          // Read Lock Status
  M16(adr) = 0xFF;                      // Revert to Read Mode
  if ((ls & 1) != 0) {                  // Check if Locked
    return (0);                         // OK
  } else {
    return (1);                         // Failed
  }
}


/*
 *  Unlock Sector
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnlockSector (unsigned long adr) {
  unsigned char ls;

  M16(adr) = 0x60;                      // Start Lock Operation
  M16(adr) = 0xD0;                      // Unlock Sector
  M16(adr) = 0x90;                      // Read Electronic Signature
  ls = M16(adr | (0x02 << 1));          // Read Lock Status
  M16(adr) = 0xFF;                      // Revert to Read Mode
  if ((ls & 1) == 0) {                  // Check if Unlocked
    return (0);                         // OK
  } else {
    return (1);                         // Failed
  }
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  unsigned char sr;

  if (UnlockSector(adr)) return (1);    // Unlock Sector

  M16(adr) = 0x20;                      // Start Erase Block Command
  M16(adr) = 0xD0;                      // Confirm Command
  do {
    sr = M16(adr);                      // Read Status Register
  } while ((sr & 0x80) == 0);           // until Flash is Ready
  M16(adr) = 0x50;                      // Clear Status Register
  M16(adr) = 0xFF;                      // Revert to Read Mode
  if (sr & 0x3A) {                      // Check for Errors
    return (1);                         // Failed
  }

  if (LockSector(adr)) return (1);      // Lock Sector

  return (0);                           // OK
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long ba = 0;
  unsigned char sr = 0;
           int  i, j, n;

  ba = adr;                             // Block Address

  if (UnlockSector(ba)) return (1);     // Unlock Sector

  n = (sz + 63) / 64;                   // Number of 64-byte Buffers
  for (i = 0; i < n; i++)  {
    do {
      M16(adr) = 0xE8;                  // Start Write to Buffer Command
    } while ((M16(adr) & 0x80) == 0);   // until Buffer available
    M16(adr) = 0x1F;                    // Word Count - 1
    for (j = 0; j < 32; j++) {
      M16(adr) = *((unsigned short *) buf);       // Write Data to Buffer
      adr += 2;                         // Next Address
      buf += 2;                         // Next Data
    }
    M16(ba) = 0xD0;                     // Confirm Command
    do {
      sr = M16(ba);                     // Read Status Register
    } while ((sr & 0x80) == 0);         // until Flash is Ready
    if (sr & 0x1A) break;               // Check for Errors
  }
  M16(ba) = 0x50;                       // Clear Status Register
  M16(ba) = 0xFF;                       // Revert to Read Mode
  if (sr & 0x1A) {                      // Check for Errors
    return (1);                         // Failed
  }

  if (LockSector(ba)) return (1);       // Lock Sector

  return (0);                           // OK
 }
