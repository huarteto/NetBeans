/******************************************************************************/
/*  This file is part of the uVision/ARM development tools                    */
/*  Copyright KEIL ELEKTRONIK GmbH 2002-2006                                  */
/******************************************************************************/
/*                                                                            */
/*  TEST.C:  Test for Flash Programming Functions                             */
/*                                                                            */
/******************************************************************************/

#include "..\..\FlashOS.H"          // FlashOS Structures


#define M8(adr) (*((volatile unsigned char  *) (adr)))

volatile int  ret;                  // Return Code
unsigned char buf[1024];            // Programming Buffer


int main (void) {
  unsigned long n;

  ret = Init(0, 12000000, 1);       // Initialize Flash Programming Functions
                                    //   Device Base Address = 0
                                    //   Clock Frequency = 12MHz

//ret |= EraseChip();               // Test Full Chip Erase

  ret |= EraseSector(0x00000000);   // Test Sector Erase ( 8kB Sector  0)
  ret |= EraseSector(0x00002000);   // Test Sector Erase ( 8kB Sector  1)
  ret |= EraseSector(0x00004000);   // Test Sector Erase ( 8kB Sector  2)
  ret |= EraseSector(0x00006000);   // Test Sector Erase ( 8kB Sector  3)
  ret |= EraseSector(0x00008000);   // Test Sector Erase ( 8kB Sector  4)
  ret |= EraseSector(0x0000A000);   // Test Sector Erase ( 8kB Sector  5)
  ret |= EraseSector(0x0000C000);   // Test Sector Erase ( 8kB Sector  6)
  ret |= EraseSector(0x0000E000);   // Test Sector Erase ( 8kB Sector  7)
  ret |= EraseSector(0x00010000);   // Test Sector Erase (64kB Sector  8)
  ret |= EraseSector(0x00020000);   // Test Sector Erase (64kB Sector  9)
  ret |= EraseSector(0x00030000);   // Test Sector Erase ( 8kB Sector 10)
  ret |= EraseSector(0x00032000);   // Test Sector Erase ( 8kB Sector 11)
  ret |= EraseSector(0x00034000);   // Test Sector Erase ( 8kB Sector 12)
  ret |= EraseSector(0x00036000);   // Test Sector Erase ( 8kB Sector 13)
  ret |= EraseSector(0x00038000);   // Test Sector Erase ( 8kB Sector 14)
  ret |= EraseSector(0x0003A000);   // Test Sector Erase ( 8kB Sector 15)
  ret |= EraseSector(0x0003C000);   // Test Sector Erase ( 8kB Sector 16)

  // Verify Erase
  for (n = 0; n < 0x0003E000; n++) {
    if (M8(n) != 0xFF) {
      ret = 1;                      // Error
      break;
    }
  }

  // Programming Test Pattern
  for (n = 0; n < 1024; n++) {
    buf[n] = (unsigned char)n;
  }

  // Test Page Programming
  ret |= ProgramPage(0x00000400, 1024, buf);

  // Verify Programming
  for (n = 0; n < 1024; n++) {
    if (M8(0x00000400 + n) != buf[n]) {
      ret = 1;                      // Error
      break;
    }
  }

  if (ret == 0) {
    // OK
  } else {
    // Error
  }

  while (1);                        // Wait forever
}
