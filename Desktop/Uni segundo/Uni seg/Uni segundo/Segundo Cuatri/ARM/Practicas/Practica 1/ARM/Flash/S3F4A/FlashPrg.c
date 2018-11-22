/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted for Samsung       */
/*               S3F4Axxx Device Series Flash                          */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures

#define M8(adr)         (*((volatile unsigned char  *) (adr)))
#define M16(adr)        (*((volatile unsigned short *) (adr)))
#define M32(adr)        (*((volatile unsigned long  *) (adr)))

#define STACK_SIZE   64                 // Stack Size

// Watchdog Registers
#define WDT_OMR         (*((volatile unsigned long  *) 0xFFE14068))

// Interleave Program Flash (IFC) Registers
#define IFC_PMSR        (*((volatile unsigned long  *) 0xFFE04058))
#define IFC_CR          (*((volatile unsigned long  *) 0xFFE04060))
#define IFC_MR          (*((volatile unsigned long  *) 0xFFE04064))
#define IFC_CSR         (*((volatile unsigned long  *) 0xFFE0406C))
#define IFC_SR          (*((volatile unsigned long  *) 0xFFE04070))
#define IFC_IER         (*((volatile unsigned long  *) 0xFFE04074))
#define IFC_IDR         (*((volatile unsigned long  *) 0xFFE04078))
#define IFC_IMR         (*((volatile unsigned long  *) 0xFFE0407C))

// Bit Constants
#define CE              (1 <<  2)
#define SE              (1 <<  1)
#define WPR             (1 <<  7)
#define STANDEN         (1 <<  4)
#define SPEEDMODE       (1 <<  2)
#define DACCESS         (1 <<  2)
#define ENDERASE        (1 <<  1)
#define ENDWR           (1 <<  0)
#define BUSY            (1 <<  8)


/************************* Flash Algorithm Functions ***************************/

/*- Init (...) -----------------------------------------------------------------
 *
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */
           
int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  switch (fnc)  {
    case 1:                             // Erase   Init
      WDT_OMR     = (0x234 << 4);       // Disable watchdog 
      IFC_MR      = (0xAC<<8) | (1<<2); // Disable write protection, high speed
      break;
    case 2:                             // Program Init
      break;
    case 3:                             // Verify  Init
      break;
  }

  return (0);                           // Success
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

  IFC_CSR = ENDERASE;                   // Clear erase status
  IFC_CR  = (0x37 << 8) | CE;           // Start full chip erase
  while (!(IFC_SR & ENDERASE));         // Wait for erase to finish

  // Full chip erase does not erase sector 0 as it expects it is used by 
  // bootloader so we have to erase it additionally
  EraseSector(0);

  return (0);                           // Success
}


/*- EraseSector (...) ----------------------------------------------------------
 *
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  unsigned int sect = adr >> 14;        // Calculate sector index

  IFC_CSR = ENDERASE;                   // Clear erase status
  IFC_CR  = (sect<<26) | (0x37<<8) | SE;// Start full chip erase
  while (!(IFC_SR & ENDERASE));         // Wait for erase to finish

  return (0);                           // Success
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

  if ((adr & 3) || (sz & 3)) {          // Address and Size must be Word aligned
    return (1);
  }

  while (sz) {                          // Loop over the Words to be programmed

    IFC_CSR  = ENDWR;                   // Clear write status
    M32(adr) = *((unsigned long *)buf); // Program a word
    while (!(IFC_SR & ENDWR));          // Wait for write to finish

    adr += 4;
    buf += 4;
    sz  -= 4;
  }

  return (0);                           // Success
}
