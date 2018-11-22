/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashPrg.C:  Flash Programming Functions adapted for External SPI Flash   */
/*               M25P64 programming with ST STM32F10x Devices                 */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures
#include "spi_flash.h"

typedef volatile unsigned char  vu8;
typedef volatile unsigned long  vu32;
typedef volatile unsigned short vu16;

#define M8(adr)  (*((vu8  *) (adr)))
#define M16(adr) (*((vu16 *) (adr)))
#define M32(adr) (*((vu32 *) (adr)))

unsigned char aux_buf[256];
unsigned long base_adr;        // Base Address


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
      base_adr = adr;
      SPI_FLASH_Init();
      break;
    case 2:                             // Program Init
      break;
    case 3:                             // Verify  Init
      break;
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

  return (0);
}


/*- EraseSector (...) ----------------------------------------------------------
 *
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  SPI_FLASH_SectorErase(adr-base_adr);

  return (0);                                   // Done
}


/*- BlankCheck (...) -----------------------------------------------------------
 *
 *  Blank Check Checks if Memory is Blank
 *    Parameter:      adr:  Block Start Address
 *                    sz:   Block Size (in bytes)
 *                    pat:  Block Pattern
 *    Return Value:   0 - OK,  1 - Failed
 */

int BlankCheck (unsigned long adr, unsigned long sz, unsigned char pat) {

  return (1);                                   // Always Force Erase
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

  SPI_FLASH_PageWrite(buf, adr-base_adr, sz);

  return (0);                                   // Done
}


/*- Verify (...) ---------------------------------------------------------------
 *
 *  Verify Flash Contents
 *    Parameter:      adr:  Start Address
 *                    sz:   Size (in bytes)
 *                    buf:  Data
 *    Return Value:   (adr+sz) - OK, Failed Address
 */

unsigned long Verify (unsigned long adr, unsigned long sz, unsigned char *buf) {

  int i;

  SPI_FLASH_BufferRead(aux_buf, adr-base_adr, sz);
  for (i = 0; i< 256; i++) {
    if (aux_buf[i] != buf[i]) 
      return (adr+i);                   // Verification Failed (return address)
  }

  return (adr+sz);                      // Done successfully
}
