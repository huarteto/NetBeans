/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for  ST M28W320CB (16-bit Bus)                        */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"         // FlashOS Structures

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

#define STACK_SIZE   64         // Stack Size


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

  M16(adr) = 0x60;              // Start Lock Operation
  M16(adr) = 0x01;              // Lock Sector
  M16(adr) = 0x90;              // Read Electornic Signature
  ls = M8(adr | 4);             // Read Lock Status
  M16(adr) = 0xFF;              // Revert to Read Mode
  if ((ls & 1) != 0) {          // Check if Locked
    return (0);                 // OK
  } else {
    return (1);                 // Failed
  }
}


/*
 *  Unlock Sector
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnlockSector (unsigned long adr) {
  unsigned char ls;

  M16(adr) = 0x60;              // Start Lock Operation
  M16(adr) = 0xD0;              // Unlock Sector
  M16(adr) = 0x90;              // Read Electornic Signature
  ls = M8(adr | 4);             // Read Lock Status
  M16(adr) = 0xFF;              // Revert to Read Mode
  if ((ls & 1) == 0) {          // Check if Unlocked
    return (0);                 // OK
  } else {
    return (1);                 // Failed
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
    sr = M8(adr);                       // Read Status Register
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
  unsigned long ba;
  unsigned char sr;
           int  i, n;

  ba = adr;                                 // Block Address

  if (UnlockSector(ba)) return (1);         // Unlock Sector

#ifdef USE_DOUBLE_WORD_PRG
  n = (sz + 3) / 4;                         // Number of Double Words
  for (i = 0; i < n; i++)  {
    M16(adr) = 0x30;                        // Start Double Word Program Command
    M16(adr) = *((unsigned short *) buf);   // Write First Word
    adr += 2;                               // Next Address
    buf += 2;                               // Next Data
    M16(adr) = *((unsigned short *) buf);   // Write Second Word
    adr += 2;                               // Next Address
    buf += 2;                               // Next Data
    do {
      sr = M8(adr);                         // Read Status Register
    } while ((sr & 0x80) == 0);             // until Flash is Ready
    if (sr & 0x1A) break;                   // Check for Errors
  }
#else
  n = (sz + 1) / 2;                         // Number of Words
  for (i = 0; i < n; i++)  {
    M16(adr) = 0x10;                        // Start Word Program Command
    M16(adr) = *((unsigned short *) buf);   // Write Word
    adr += 2;                               // Next Address
    buf += 2;                               // Next Data
    do {
      sr = M8(adr);                         // Read Status Register
    } while ((sr & 0x80) == 0);             // until Flash is Ready
    if (sr & 0x1A) break;                   // Check for Errors
  }
#endif

  M16(adr) = 0x50;                          // Clear Status Register
  M16(adr) = 0xFF;                          // Revert to Read Mode
  if (sr & 0x1A) {                          // Check for Errors
    return (1);                             // Failed
  }

  if (LockSector(ba)) return (1);           // Lock Sector

  return (0);                               // OK
}
