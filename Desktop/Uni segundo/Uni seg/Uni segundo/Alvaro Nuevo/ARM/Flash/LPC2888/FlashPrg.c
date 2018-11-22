/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for NXP LPC2888 Internal Flash                        */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

#define STACK_SIZE      64              // Stack Size

unsigned long or_mask[4] = {0x00000000, 0xFFFFFF00, 0xFFFF0000, 0xFF000000};

// Flash Memory Interface Registers
#define F_CTRL          (*((volatile unsigned long  *) 0x80102000))
#define F_STAT          (*((volatile unsigned long  *) 0x80102004))
#define F_PROG_TIME     (*((volatile unsigned long  *) 0x80102008))
#define F_WAIT          (*((volatile unsigned long  *) 0x80102010))
#define F_CLK_TIME      (*((volatile unsigned long  *) 0x8010201C))
#define F_INTEN_CLR     (*((volatile unsigned long  *) 0x80102FD8))
#define F_INTEN_SET     (*((volatile unsigned long  *) 0x80102FDC))
#define F_INT_STAT      (*((volatile unsigned long  *) 0x80102FE0))
#define F_INTEN         (*((volatile unsigned long  *) 0x80102FE4))
#define F_INT_CLR       (*((volatile unsigned long  *) 0x80102FE8))
#define F_INT_SET       (*((volatile unsigned long  *) 0x80102FEC))
#define FLASH_PD        (*((volatile unsigned long  *) 0x80005030))
#define FLASH_INIT      (*((volatile unsigned long  *) 0x80005034))

// Flash Command Bits
#define FC_CS           (1<< 0)
#define FC_FUNC         (1<< 1)
#define FC_WEN          (1<< 2)
#define FC_RD_LATCH     (1<< 5)
#define FC_PROTECT      (1<< 7)
#define FC_SET_DATA     (1<<10)
#define FC_RSSL         (1<<11)
#define FC_PROG_REQ     (1<<12)
#define FC_CLR_BUF      (1<<14)
#define FC_LOAD_REQ     (1<<15)

// Flash Status Bits
#define FS_DONE         (1<< 0)
#define FS_PROGGNT      (1<< 1)
#define FS_RDY          (1<< 2)
#define FS_ERR          (1<< 5)

// Flash Program Time Bits
#define FPT_ENABLE      (1<<15)


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
  unsigned long i;

  // Wait for Flash to become ready
  while (!(F_STAT & FS_RDY));

  // Unprotect all sectors
  for (i = 0; i < 0x0F0000; i += 0x10000) {
    M32(0x10400000+i) = 2;
    F_CTRL = FC_CS | FC_FUNC | FC_WEN | FC_PROTECT | FC_LOAD_REQ;
  }
  for (i = 0x0; i < 0x10000; i += 0x2000) {
    M32(0x104F0000+i) = 2;
    F_CTRL = FC_CS | FC_FUNC | FC_WEN | FC_PROTECT | FC_LOAD_REQ;
  }

  // Wait for Flash to become ready
  while (!(F_STAT & FS_RDY));

  // Enable 66 kHz clock for Flash Module
  F_CLK_TIME = 60;

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
  unsigned long i;

  // Set timer register for erase
  F_PROG_TIME = 0x7FFF | FPT_ENABLE;

  // Select all sectors except last for erasure
  for (i = 0; i < 0x0F0000; i += 0x10000) {
    M32(0x10400000+i) = 0;
    F_CTRL = FC_CS | FC_WEN | FC_PROTECT | FC_LOAD_REQ;
  }
  for (i = 0x0; i < 0x10000-0x02000; i += 0x02000) {
    M32(0x104F0000+i) = 0;
    F_CTRL = FC_CS | FC_WEN | FC_PROTECT | FC_LOAD_REQ;
  }

  // Start last sector erase
  M32(0x104FE000) = 0;
  F_CTRL   = FC_CS | FC_PROTECT | FC_PROG_REQ;

  while (!(F_STAT & FS_DONE));          // Wait command to finish

  return (0);                           // Done
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  // Set timer register for erasing
  F_PROG_TIME = 9375 | FPT_ENABLE;

  // Issue sector erase command
  M32(adr) = 0;
  F_CTRL   = FC_CS | FC_PROTECT | FC_PROG_REQ;

  while (!(F_STAT & FS_DONE));          // Wait command to finish

  return (0);                           // Done
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long i;

  // Preset data latches
  F_CTRL   = FC_CS | FC_FUNC | FC_WEN | FC_SET_DATA;
  F_CTRL   = FC_CS | FC_FUNC | FC_WEN;

  // Set timer register for programming
  F_PROG_TIME = 800 | FPT_ENABLE;

  // If size is not whole number of words, 
  // fill rest of last word with 0xFF
  if ((sz%4) != 0) {
    *((unsigned long *)buf+(sz/4)) |= or_mask[sz%4];
  }

  // Load data to data latches
  for (i = 0; i < ((sz+3)/4); i++)  {
    M32(adr) = *((unsigned long *)buf);
    buf += 4;
    adr += 4;
  }
  // Load remaining bytes to full page (512) with 0xFF
  for (i = ((sz+3)/4); i < (512/4); i++) {
    M32(adr) = 0xFFFFFFFF;
    adr += 4;
  }

  // Issue program command
  F_CTRL   = FC_CS | FC_FUNC | FC_PROTECT | FC_PROG_REQ;

  while (!(F_STAT & FS_DONE));          // Wait command to finish

  // Disable timer
  F_PROG_TIME = 0;

  return (0);                           // Done successfully
}
