/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for AT91SAM7A3 (ONCHIP)                               */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures
#include <AT91SAM7A3.H>

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
  unsigned long clkus;

  base_adr = adr;

  // Calculate Flash Microsecond Cycle Number - Approximate (no Library Code)
//clkus = (1074*(clk >> 10)) >> 20;            // Master Clock Cycles in 1.0us
  clkus = (1611*(clk >> 10)) >> 20;            // Master Clock Cycles in 1.5us

  // Set Flash Microsecond Cycle Number
  // Set Flash Waite State to max. (Single Cycle Access at Up to 30 MHz)
  AT91C_BASE_MC->MC_FMR = ((AT91C_MC_FMCN) & (clkus << 16)) | AT91C_MC_FWS_3FWS;

  // Disable Watchdog
  AT91C_BASE_WDTC->WDTC_WDMR = AT91C_WDTC_WDDIS;

  // Pointer to PMC
  pPMC = AT91C_BASE_PMC;

  // Enable the Main Oscillator:
  // SCK Period = 1/32768 = 30.51 us
  // Start-up Time = 8 * 6 / SCK = 56 * 30.51 = 1,46484375 ms
  pPMC->PMC_MOR = ((AT91C_CKGR_OSCOUNT & (0x06 << 8) | AT91C_CKGR_MOSCEN));

  // Wait the Start-up Time
  while(!(pPMC->PMC_SR & AT91C_PMC_MOSCS));

  // Select Main Clock
  pPMC->PMC_MCKR = AT91C_PMC_CSS_MAIN_CLK;

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
  return (0);
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {

  // Erase All Command
  AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_ERASE_ALL;

  // Wait until the end of Command
  while ((AT91C_BASE_MC->MC_FSR & AT91C_MC_EOP) != AT91C_MC_EOP);

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
  AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_UNLOCK |
                         (AT91C_MC_PAGEN & (page << 8));

  // Wait until the end of Command
  while ((AT91C_BASE_MC->MC_FSR & AT91C_MC_EOL) != AT91C_MC_EOL);

  // Copy to the Write Buffer
  for (sz = (sz + 3) & ~3; sz; sz -= 4, buf += 4) {
	*Flash++ = *((unsigned long *)buf);
  }
  //* Start Programming Command
  AT91C_BASE_MC->MC_FCR = AT91C_MC_CORRECT_KEY | AT91C_MC_FCMD_START_PROG |
                         (AT91C_MC_PAGEN & (page << 8));

  // Wait until the end of Command
  while ((AT91C_BASE_MC->MC_FSR & AT91C_MC_EOP) != AT91C_MC_EOP);

  // Check for Errors
  if (AT91C_BASE_MC->MC_FSR & (AT91C_MC_PROGE | AT91C_MC_LOCKE)) return (1);

  return (0);
}
