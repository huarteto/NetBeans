/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Flash Programming Functions for AT91SAM9XE internal Embedded */
/*               Flash                                                        */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures
#include <AT91SAM9XE512.H>

#define M8(adr)      (*((volatile unsigned char  *) (adr)))
#define M16(adr)     (*((volatile unsigned short *) (adr)))
#define M32(adr)     (*((volatile unsigned long  *) (adr)))

#define FKEY         ((unsigned int) 0x5A << 24)
#define PAGE_SIZE    512

#define STACK_SIZE   64                 // Stack Size


unsigned long base_adr;                 // Base Address


/*- Init (...) -----------------------------------------------------------------
 *
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  base_adr = adr;
  if ((*AT91C_PMC_MCKR & AT91C_PMC_CSS_MAIN_CLK) != AT91C_PMC_CSS_MAIN_CLK) {

    // Set Flash Waite State to 4
    *AT91C_EFC_FMR = AT91C_EFC_FWS_3WS;

    // Enable the Main Oscillator
    // SCK Period = 1/32768 = 30.52 us
    // Start-up Time = 8*15/SCK = 3.9321 ms
    *AT91C_PMC_MOR = (0x0F << 8) | AT91C_CKGR_MOSCEN;

    // Wait the Start-up Time
    while(!(*AT91C_PMC_SR & AT91C_PMC_MOSCS));

    // Select Main Clock
    *AT91C_PMC_MCKR = AT91C_PMC_CSS_MAIN_CLK;

    // Wait for Clock ready
    while (!(*AT91C_PMC_SR & AT91C_PMC_MCKRDY));
  }

  if ((*AT91C_WDTC_WDMR & AT91C_WDTC_WDDIS) != AT91C_WDTC_WDDIS) {
    // Disable Watchdog
    *AT91C_WDTC_WDMR = AT91C_WDTC_WDDIS;
  }

  return (0);
}


/*- UnInit (...) ---------------------------------------------------------------
 *
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  if (fnc == 2) {
    // Set GPNVM bit 3 for booting from Flash
    *AT91C_EFC_FCR = FKEY | AT91C_EFC_FCMD_SFB | (3 << 8);
  }

  return (0);
}


/*- EraseChip (...) ------------------------------------------------------------
 *
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {

  // Erase All Command
  *AT91C_EFC_FCR = FKEY | AT91C_EFC_FCMD_EA;

  // Wait until the end of Command
  while (!(*AT91C_EFC_FSR & AT91C_EFC_FRDY_S));

  return (0);
}


/*- EraseSector (...) ----------------------------------------------------------
 *
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  return (0);                  // Automatic Erase during Program Cycle
}


/*- ProgramPage (...) ----------------------------------------------------------
 *
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long  page;

  page = (adr - base_adr) / PAGE_SIZE;

  // Copy to the Write Buffer
  for (sz = (sz + 3) & ~3; sz; sz -= 4, adr += 4, buf += 4) {
    M32(adr) = *((unsigned long *)buf);
  }
  // Start Erase Page and Write Page Command
  *AT91C_EFC_FCR = FKEY | AT91C_EFC_FCMD_EWP | (page << 8);

  // Wait until the end of Command
  while (!(*AT91C_EFC_FSR & AT91C_EFC_FRDY_S));

  // Check for Errors
  if (*AT91C_EFC_FSR & (AT91C_EFC_FCMDE | AT91C_EFC_LOCKE)) 
    return (1);

  return (0);
}
