/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for NXP SJA2510                                       */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures

#include <SJA2510.H>

#define M8(adr)      (*((volatile unsigned char  *) (adr)))
#define M16(adr)     (*((volatile unsigned short *) (adr)))
#define M32(adr)     (*((volatile unsigned long  *) (adr)))

#define FLASH_BASE   0x20000000         // Flash Base Address

#define STACK_SIZE   64                 // Stack Size


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  int i;

  // Setup clock to use crystal 10 MHz and generate SYS clock of 80MHz
  SYS_CLK_CONF = CLK_SEL_XTAL;
  PLL_CONTROL  = P23EN + PLL_PD;
  PLL_CONTROL  = PLL_XTAL_SEL + (15 << MSEL_SHIFT) + P23EN + PLL_PD;
  PLL_CONTROL  = PLL_XTAL_SEL + (15 << MSEL_SHIFT) + P23EN;
  while (!(PLL_STATUS & PLL_LOCK   ));  // Wait for PLL Lock
  while (!(RDET       & PLL_PRESENT));  // Wait for Clock Present
  SYS_CLK_CONF = CLK_SEL_PLL + AUTOBLK + DIV2;
  FCTR         = FS_CS + FS_DCR + FS_WPB;
  FBWST        = CACHE2EN + SPECALWAYS + WST4;
  FCRA         = ((80000000/3)/66000)-1;

  // Unprotect all sectors
  for (i = 0; i < 0x10000; i += 0x2000) {
    M32(i+FLASH_BASE)  = 2;
    FCTR               = FS_CS + FS_WRE + FS_WEB + FS_DCR + FS_WPB + FS_LOADREQ;
  }

#ifdef SIZE_512kB
  for (i = 0x10000; i < 0x80000; i += 0x10000) {
    M32(i+FLASH_BASE)  = 2;
    FCTR               = FS_CS + FS_WRE + FS_WEB + FS_DCR + FS_WPB + FS_LOADREQ;
  }
#elif  SIZE_768kB
  for (i = 0x10000; i < 0xC0000; i += 0x10000) {
    M32(i+FLASH_BASE)  = 2;
    FCTR               = FS_CS + FS_WRE + FS_WEB + FS_DCR + FS_WPB + FS_LOADREQ;
  }
#endif

  return (0);   // Success
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

  FMC_INT_CLR_STATUS = 0x01;            // Clear End of Erase flag

  // Start erase procedure
  M32(adr)  = 0;
  FCTR      = FS_CS +          FS_WEB + FS_DCR + FS_WPB + FS_LOADREQ;

  // Set Erase Timer value
  FPTR      = ((105 * 80000) / 512) | (1 << 15);
  
  // Triger sector erase
  FCTR      = FS_CS +                 + FS_DCR + FS_WPB + FS_PROGREQ;

  while (!(FMC_INT_STATUS & 0x01));     // Wait for erase to finish

  return (0);   // Success
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {

  unsigned char rest, sz_add;

  FMC_INT_CLR_STATUS = 0x02;          // Clear End of Burn flag

  // Preset data latches
  FCTR      = FS_CS + FS_WRE + FS_WEB + FS_DCR + FS_PDL;
  FCTR      = FS_CS + FS_WRE + FS_WEB + FS_DCR;

  if (sz >= 512) {
    rest   = 0;
    sz_add = 0;
    sz     = 512;
  }  else {
    rest   = sz & 0x03;
    sz_add = (16 - (sz & 0x0F)) >> 2;
    sz    -= rest;
  }

  // Load whole words to data latches
  while (sz) {
    M32(adr) = *((unsigned long *)buf);
    adr += 4;
    buf += 4;
    sz  -= 4;
  }

  // Load remaining word to data latches (if rest != 0)
  if (rest != 0) {
    M32(adr) = *((unsigned long *)buf) | (0xFFFFFFFF << (rest << 3));
    adr     += 4;
  }

  // Load additional words if number of loaded words is not a round number of 4 words
  while (sz_add--) {
    M32(adr) = 0xFFFFFFFF;
    adr     += 4;
  }

  // Set Program Timer value
  FPTR      = ((105 * 800) / 512) | (1 << 15);

  // Triger programming
  FCTR      = FS_CS + FS_WRE +          FS_DCR + FS_WPB + FS_PROGREQ;

  while (!(FMC_INT_STATUS & 0x02));     // Wait for burn to finish

  return (0);   // Success
}
