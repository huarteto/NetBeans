/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for EFM32 Flash                                       */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        /* FlashOS Structures */
#include "efm32.h"

#define EFM32_GEOMETRY_FLASH_PAGE_SIZE      0x00000200
#define EFM32_FLASH_PROGRAM_WORD_TIMEOUT    10000000
#define EFM32_GEOMETRY_FLASH_BASE           0x0

/* The size of the flash in the device */
uint32_t flashSize;

/*****************************************************************************
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 ****************************************************************************/
int Init(unsigned long adr, unsigned long clk, unsigned long fnc)
{
  uint32_t ramSize;

  /* Unlock the MSC */
  MSC->LOCK = MSC_UNLOCK_CODE;

  /* Read the flash size from the Device Information Page */
  flashSize = (DEVINFO->MSIZE & _DEVINFO_MSIZE_FLASH_MASK)
              >> _DEVINFO_MSIZE_FLASH_SHIFT;

  ramSize = (DEVINFO->MSIZE & _DEVINFO_MSIZE_SRAM_MASK)
             >> _DEVINFO_MSIZE_SRAM_SHIFT;

  /* Revision A chips have a bug that causes ramSize and flashSize
     to be swapped. */
  if (ramSize > flashSize)
  {
  	flashSize = ramSize;
  }

  /* Convert to Bytes */
  flashSize = flashSize << 10;

  /* Enable writing in MSC */
  MSC->WRITECTRL |= MSC_WRITECTRL_WREN;

  return(0);
}


/*****************************************************************************
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 ****************************************************************************/
int UnInit(unsigned long fnc)
{
  /* Disable write in MSC */
  MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
  return(0);
}


/*****************************************************************************
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 ****************************************************************************/
int EraseChip(void)
{
  uint32_t addr;

  for (addr = EFM32_GEOMETRY_FLASH_BASE; addr <= flashSize;
       addr += EFM32_GEOMETRY_FLASH_PAGE_SIZE)
  {
    if (EraseSector(addr) != 0)
      return(1);
  }
  return(0);
}

/*****************************************************************************
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 ****************************************************************************/
int EraseSector(unsigned long adr)
{
  int timeOut = EFM32_FLASH_PROGRAM_WORD_TIMEOUT;

  /* Check that the address is within bounds */
  if (adr > flashSize)
  {
    return(1);
  }

  /* The address must be block aligned. */
  if ((adr) & (EFM32_GEOMETRY_FLASH_PAGE_SIZE - 1))
  {
    return(1);
  }

  /* Load address */
  MSC->ADDRB     = adr;
  MSC->WRITECMD |= MSC_WRITECMD_LADDRIM;

  /* Check for invalid address */
  if (MSC->STATUS & MSC_STATUS_INVADDR)
  {
    MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
    return(1);
  }

  /* Send Erase Page command */
  MSC->WRITECMD |= MSC_WRITECMD_ERASEPAGE;

  /* Waiting for the write to complete */
  while ((MSC->STATUS & MSC_STATUS_BUSY) && (timeOut != 0))
  {
    timeOut--;
  }

  /* Check to see if the erase was aborted due to locked page */
  if (MSC->STATUS & MSC_STATUS_LOCKED)
  {
    return 1;
  }

  /* Return 1 if there was a timeout */
  if (timeOut == 0)
  {
    return 1;
  }

  return(0);
}

/*****************************************************************************
 *  Write a single word (32 bit) to Flash Memory
 *    Parameter:      adr:  Sector Address
 *                    data: Data to write
 *    Return Value:   0 - OK,  1 - Failed
 ****************************************************************************/
uint32_t WriteWord(uint32_t adr, uint32_t data)
{
  int timeOut = EFM32_FLASH_PROGRAM_WORD_TIMEOUT;

  /* Check that the address is within bounds */
  if (adr > flashSize)
  {
    return(1);
  }

  /* Load address, do checks, trigger write once */
  MSC->ADDRB     = adr;
  MSC->WRITECMD |= MSC_WRITECMD_LADDRIM;

  /* Check for invalid address */
  if (MSC->STATUS & MSC_STATUS_INVADDR)
  {
    MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
    return 1;
  }
  /* Check that the MSC is ready to receive a new word */
  if (!(MSC->STATUS & MSC_STATUS_WDATAREADY))
  {
    MSC->WRITECTRL &= ~MSC_WRITECTRL_WREN;
    return 1;
  }

  /* Load data into Write Data register */
  MSC->WDATA = data;
  /* Trigger write once */
  MSC->WRITECMD |= MSC_WRITECMD_WRITEONCE;

  /* Waiting for the write to complete */
  while ((MSC->STATUS & MSC_STATUS_BUSY) && (timeOut != 0))
  {
    timeOut--;
  }

  /* Check to see if the write was aborted due to locked page */
  if (MSC->STATUS & MSC_STATUS_LOCKED)
  {
    return 1;
  }

  /* Return */
  if (timeOut == 0)
  {
    return 1;
  }
  return 0;
}

/*****************************************************************************
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 ****************************************************************************/
int ProgramPage(unsigned long adr, unsigned long sz, unsigned char *buf)
{
  uint32_t count   = 0;
  uint32_t ret     = 0;
  uint32_t *buffer = (uint32_t *) buf;

  /* Block must be word aligned */
  if ((adr & 3))
  {
    return(1);
  }

  /* Iterate across the buffer, write one word per iteration. */
  while ((count < sz) && (ret == 0))
  {
    ret = WriteWord(adr, *buffer);
    buffer++;
    count += 4;
    adr   += 4;
  }

  return(ret);
}
