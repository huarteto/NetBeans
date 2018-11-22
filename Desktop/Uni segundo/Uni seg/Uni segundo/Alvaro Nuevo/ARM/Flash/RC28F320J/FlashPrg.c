/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for Intel E/RC28F320J (16-bit Bus)                    */
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
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  unsigned char sr;

  M8(adr) = 0x20;               // Start Erase Block Command
  M8(adr) = 0xD0;               // Confirm Command
  do {
    sr = M8(adr);               // Read Status Register
  } while ((sr & 0x80) == 0);   // until Flash is Ready
  M8(adr) = 0x50;               // Clear Status Register
  M8(adr) = 0xFF;               // Revert to Read Mode
  if (sr & 0x3A) {              // Check for Errors
    return (1);                 // Failed
  } else {
    return (0);                 // OK
  }
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
           int  i, j, n;

  n = (sz + 31) / 32;                           // Number of 32-byte Buffers
  for (i = 0; i < n; i++)  {
    ba = adr;                                   // Block Address
    do {
      M8(adr) = 0xE8;                           // Start Write to Buffer Command
    } while ((M8(adr) & 0x80) == 0);            // until Buffer available
    M8(adr) = 0x0F;                             // Word Count - 1
    for (j = 0; j < 16; j++) {
      M16(adr) =  *((unsigned short *) buf);    // Write Data to Buffer
      adr += 2;                                 // Next Address
      buf += 2;                                 // Next Data
    }
    M8(ba) = 0xD0;                              // Confirm Command
  }
  do {
    sr = M8(ba);                                // Read Status Register
  } while ((sr & 0x80) == 0);                   // until Flash is Ready
  M8(ba) = 0x50;                                // Clear Status Register
  M8(ba) = 0xFF;                                // Revert to Read Mode
  if (sr & 0x1A) {                              // Check for Errors
    return (1);                                 // Failed
  } else {
    return (0);                                 // OK
  }
}
