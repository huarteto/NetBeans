/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for Luminary Micro LM3Sxxx                            */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#include <LM3Sxxx.H>


#define STACK_SIZE   64        // Stack Size


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  // Set the Number of Clocks per microsecond for the Flash Controller
  // Approximate division by 1000000 (no Library Code)
  HWREG(FLASH_USECRL) = ((1074*(clk >> 10)) >> 20) - 1;  // clk / 1000000 - 1;
  // Clear the BA bit in RMCTL by writing a 1 to this location. 
  HWREG(FLASH_RMCTL)  = 0x00000001;
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

  // Clear the Flash Access Interrupt
  HWREG(FLASH_FCMISC) = FLASH_FCMISC_ACCESS;

#if 0

  // Mass Erase
  HWREG(FLASH_FMC) = FLASH_FMC_WRKEY | FLASH_FMC_MERASE;

  // Wait until Erase is done
  while (HWREG(FLASH_FMC) & FLASH_FMC_MERASE);

#else
  // Mass Erase according to Errata for 256kB Flash

  // Mass Erase Part 1
  HWREG(FLASH_FMA) = 0x00000000;
  HWREG(FLASH_FMC) = FLASH_FMC_WRKEY | FLASH_FMC_MERASE;

  // Wait until Erase is done
  while (HWREG(FLASH_FMC) & FLASH_FMC_MERASE);

  // Mass Erase Part 2
  HWREG(FLASH_FMA) = 0x00020000;
  HWREG(FLASH_FMC) = FLASH_FMC_WRKEY | FLASH_FMC_MERASE;

  // Wait until Erase is done
  while (HWREG(FLASH_FMC) & FLASH_FMC_MERASE);

#endif

  // Check Access Violation
  if (HWREG(FLASH_FCRIS) & FLASH_FCRIS_ACCESS) {
    return (1);
  }

  return (0);   // Success
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  // Address must be Block aligned
  if (adr & (FLASH_ERASE_SIZE - 1)) {
    return (1);
  }

  // Clear the Flash Access Interrupt
  HWREG(FLASH_FCMISC) = FLASH_FCMISC_ACCESS;

  // Erase the Block
  HWREG(FLASH_FMA) = adr;
  HWREG(FLASH_FMC) = FLASH_FMC_WRKEY | FLASH_FMC_ERASE;

  // Wait until Erase is done
  while (HWREG(FLASH_FMC) & FLASH_FMC_ERASE);

  // Check Access Violation
  if (HWREG(FLASH_FCRIS) & FLASH_FCRIS_ACCESS) {
    return (1);
  }

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

  // Address and Size must be Word aligned
  if ((adr & 3) || (sz & 3)) {
    return (1);
  }

  // Clear the Flash Access Interrupt
  HWREG(FLASH_FCMISC) = FLASH_FCMISC_ACCESS;
  // See if this device has a write buffer.
  if(HWREG(SYSCTL_NVMSTAT) & SYSCTL_NVMSTAT_FWB)
  {

    // Loop over the words to be programmed

    while(sz)
    {
      //
      // Set the address of this block of words.
      //
      HWREG(FLASH_FMA)        = adr & ~(0x7f);
      //
      // Loop over the words in this 32-word block.
      //
      while(((adr & 0x7c) || (HWREG(FLASH_FWBVAL) == 0)) &&
            (sz != 0))
      {
        //
        // Write this word into the write buffer.
        //
        HWREG(FLASH_FWBN + (adr & 0x7c)) = *((unsigned long *)buf);
        buf += 4;
        adr += 4;
        sz  -= 4;
      }
    
      //
      // Program the contents of the write buffer into flash.
      //
      HWREG(FLASH_FMC2) = FLASH_FMC2_WRKEY | FLASH_FMC2_WRBUF;
    
      //
      // Wait until the write buffer has been programmed.
      //
      while(HWREG(FLASH_FMC2) & FLASH_FMC2_WRBUF)
      {
      }
    }
  }
  else {
    // Loop over the Words to be programmed
    while (sz) {
  
      // Program Word
      HWREG(FLASH_FMA) = adr;
      HWREG(FLASH_FMD) = *((unsigned long *)buf);
      HWREG(FLASH_FMC) = FLASH_FMC_WRKEY | FLASH_FMC_WRITE;
  
      // Wait unitl Word has been programmed
      while (HWREG(FLASH_FMC) & FLASH_FMC_WRITE);
  
      // Prepeare Next Word
      adr += 4;
      buf += 4;
      sz  -= 4;
  
    }
  }

  // Check Access Violation
  if (HWREG(FLASH_FCRIS) & FLASH_FCRIS_ACCESS) {
    return (1);
  }

  return (0);   // Success
}
