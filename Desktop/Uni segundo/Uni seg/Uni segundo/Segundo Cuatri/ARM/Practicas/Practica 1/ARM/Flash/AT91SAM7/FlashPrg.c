/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2003-2009 Keil Software. All rights reserved.        */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for AT91SAM7L Flash                                   */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures
#include <AT91SAM7L128.H>

#define AT91C_MC_CORRECT_KEY   ((unsigned int) 0x5A << 24)
#define FLASH_PAGE_SIZE_BYTE   256

#define STACK_SIZE   64        // Stack Size


unsigned long base_adr;        // Base Address


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
  AT91PS_PMC    pPMC;

  base_adr = adr;

  // Set Flash Waite State to 3
  AT91C_BASE_MC->MC_FMR = AT91C_MC_FWS_3FWS;

  // Disable Watchdog
  AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;

  // Pointer to PMC
  pPMC = AT91C_BASE_PMC;

  // Wait for Clock ready
  while (!(pPMC->PMC_SR & AT91C_PMC_MCKRDY));

  return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  if (fnc == 2) {
    // Set GPNVM bit 1 for booting from Flash
    AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_SFB | (1 << 8);
  }

  return (0);
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {

  // Erase All Command
  AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_EA;

  // Wait until the end of Command
  while (!(AT91C_BASE_MC->MC_FSR & AT91C_MC_FRDY_S));

  return (0);
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  return (0);                  // Automatic Erase during Program Cycle
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long   page;
  unsigned long * Flash;

  Flash = (unsigned long *)adr;
  page  = (adr - base_adr) / FLASH_PAGE_SIZE_BYTE;

  // Unlock Page Command
  AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | (page << 8) | AT91C_MC_FCMD_CLB;

  // Wait until the end of Command
  while (!(AT91C_BASE_MC->MC_FSR & AT91C_MC_FRDY_S));

  // Copy to the Write Buffer
  for (sz = (sz + 3) & ~3; sz; sz -= 4, buf += 4) {
	*Flash++ = *((unsigned long *)buf);
  }
  // Start Programming Command
  AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | (page << 8) | AT91C_MC_FCMD_WP;

  // Wait until the end of Command
  while (!(AT91C_BASE_MC->MC_FSR & AT91C_MC_FRDY_S));

  // Check for Errors
  if (AT91C_BASE_MC->MC_FSR & (AT91C_MC_FCMDE | AT91C_MC_LOCKE)) return (1);

  return (0);
}
