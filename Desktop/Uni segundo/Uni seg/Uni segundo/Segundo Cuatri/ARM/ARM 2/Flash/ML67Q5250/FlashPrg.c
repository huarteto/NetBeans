/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted for               */
/*               OKI ML67Q5250 Flash                                   */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

// Flash Control Register
#define FLACON   (*((volatile unsigned long  *) 0xB7000200))

// Flash Control Bits
#define	FLWE     0x00000001    // Flash Write Enable
#define	FWEND    0x00000002    // Flash Write Ended
#define BUSY     0x00000004    // Busy
#define BUSYNVW  0x00000008    // BusyNVW
#define SEQMD    0x00000010    // Security Enable
#define SEQM     0x00000020    // Security Bit
#define FLRST    0x00000040    // Flash Reset

#define STACK_SIZE   64        // Stack Size


unsigned long base_adr;


/*
 *  Delay Function
 *    Parameter:      None
 *    Return Value:   None
 */

void Delay (void)  {
  __asm { NOP };
  __asm { NOP };
  __asm { NOP };
  __asm { NOP };
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
  base_adr = adr;
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

  FLACON = FLWE | FWEND;           /* Enable Flash and Clear End Bit */
  Delay();

  /* Start Chip Erase Command */
  M8(base_adr + 0x15554) = 0xAA;
  M8(base_adr + 0x0AAA8) = 0x55;
  M8(base_adr + 0x15554) = 0x80;
  M8(base_adr + 0x15554) = 0xAA;
  M8(base_adr + 0x0AAA8) = 0x55;
  M8(base_adr + 0x15554) = 0x10;

  while ((FLACON & 0x0E) != 0x02); /* Wait until Erase completed */

  return (0);
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  FLACON = FLWE | FWEND;           /* Enable Flash and Clear End Bit */
  Delay();

  /* Start Erase Sector Command */
  M8(base_adr + 0x15554) = 0xAA;
  M8(base_adr + 0x0AAA8) = 0x55;
  M8(base_adr + 0x15554) = 0x80;
  M8(base_adr + 0x15554) = 0xAA;
  M8(base_adr + 0x0AAA8) = 0x55;
  M8(adr) = 0x30;

  while ((FLACON & 0x0E) != 0x02); /* Wait until Erase completed */

  return (0);
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  int i;

  for (i = 0; i < ((sz+3)/4); i++)  {
    FLACON = FLWE | FWEND;         /* Enable Flash and Clear End Bit */
    Delay();

    /* Start Program Command */
    M8(base_adr + 0x15554) = 0xAA;
    M8(base_adr + 0x0AAA8) = 0x55;
    M8(base_adr + 0x15554) = 0xA0;
    M8(adr++) = *buf++;
    M8(adr++) = *buf++;
    M8(adr++) = *buf++;
    M8(adr++) = *buf++;

    while ((FLACON & 0x0E) != 0x02);
  }
  return (0);
}
