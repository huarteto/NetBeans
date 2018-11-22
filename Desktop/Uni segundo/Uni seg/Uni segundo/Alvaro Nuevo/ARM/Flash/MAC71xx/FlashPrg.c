/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                                */
/******************************************************************************/
/*                                                                            */
/*  FlashDev.C:  Flash Programming Functions adapted                          */
/*               for MAC71xx Program Flash                                    */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures

#include <MAC7100.h>                    // Include vendors register definitons

#define M8(adr)    (*((volatile unsigned char  *) (adr)))
#define M16(adr)   (*((volatile unsigned short *) (adr)))
#define M32(adr)   (*((volatile unsigned long  *) (adr)))


#define STACK_SIZE   64                 // Stack Size

// Flash programming interface arrays
#define PROG_FLASH  0xFC100000          // Program Flash array address
#define DATA_FLASH  0xFE000000          // Data    Flash array address

// Flash Commands
#define CMD_BCHK    0x05                // Blank Check
#define CMD_PEV     0x06                // Page Erase Verify
#define CMD_PRG     0x20                // Program
#define CMD_PRGERS  0x40                // Page Erase
#define CMD_MASSERS 0x41                // Mass Erase
#define CMD_DSIG    0x65                // Data Signature


unsigned long base_adr;


/*
 *  Wait flash interface to become ready and clear status register
 *    Parameter:
 *    Return Value:
 */

void WaitFlashReady (void) {

  while (!(CFM_CFMUSTAT & CFM_CBEIF));  // Wait for command buffer empty

  // If protect violation or acces error active, clear error flag
  if (CFM_CFMUSTAT & (CFM_ACCERR | CFM_PVIOL))
    CFM_CFMUSTAT |= (CFM_ACCERR | CFM_PVIOL);
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  CFM_CFMCLKD = (CFM_DIVLD | CFM_DIV(19)); // Setup Flash clock

  if ((CFM_CFMCLKD & 0x3F) != CFM_DIV(19)) // If clock not setup correctly
    return (1);                            // Return fail

  CFM_CFMPROT   = 0;                    // Unprotect all sectors in Prog flash
  CFM_CFMDFPROT = 0;                    // Unprotect all sectors in Data flash
  CFM_CFMSACC   = 0;                    // Logical sectors placed to
                                        // unrestricted address space

  CFM_CFMSEC   |= CFM_SEC(2);           // Unsecure Flash memory

  base_adr = adr;
  return (0);                           // Done successfully
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  return (0);                           // Done successfully
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {

  WaitFlashReady();                     // Wait flash to become ready

  M32(PROG_FLASH) = 0;                  // Dummy write to prog flash
  CFM_CFMCMD      = CMD_MASSERS;        // Issue Mass Erase Command
  CFM_CFMUSTAT   |= CFM_CBEIF;          // Activate command

  // If protect violation or acces error active, clear error flag
  if (CFM_CFMUSTAT & (CFM_ACCERR | CFM_PVIOL))  {
    CFM_CFMUSTAT |= (CFM_ACCERR | CFM_PVIOL);
    return (1);                         // Return fail
  }

  while (!(CFM_CFMUSTAT & CFM_CBEIF));  // Wait for command buffer empty

  CFM_CFMSEC |= CFM_SEC(2);             // Unsecure Flash memory

  while (!(CFM_CFMUSTAT & CFM_CCIF ));  // Wait for command to finish

  return (0);                           // Done successfully
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  WaitFlashReady();                     // Wait flash to become ready

  M32(PROG_FLASH + adr - base_adr) = 0; // Dummy write to prog flash

  CFM_CFMCMD    = CMD_PRGERS;           // Issue Page Erase Command
  CFM_CFMUSTAT |= CFM_CBEIF;            // Activate command

  // If protect violation or acces error active, clear error flag
  if (CFM_CFMUSTAT & (CFM_ACCERR | CFM_PVIOL))  {
    CFM_CFMUSTAT |= (CFM_ACCERR | CFM_PVIOL);
    return (1);                         // Return fail
  }

  while (!(CFM_CFMUSTAT & CFM_CBEIF));  // Wait for command buffer empty

  CFM_CFMSEC |= CFM_SEC(2);             // Unsecure Flash memory

  while (!(CFM_CFMUSTAT & CFM_CCIF ));  // Wait for command to finish

  return (0);                           // Done successfully
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {

  unsigned int i;

	adr = PROG_FLASH + adr - base_adr;  // Address for access (in prog flash)

  WaitFlashReady();                   // Wait flash to become ready

  for (i = 0; i < ((sz+3)/4); i++)  {
    M32(adr) = *((unsigned int *)buf);// Write data
    CFM_CFMCMD    = CMD_PRG;          // Issue Program Command
    CFM_CFMUSTAT |= CFM_CBEIF;        // Activate command
    adr += 4;
    buf += 4;

    // If protect violation or acces error active, clear error flag
    if (CFM_CFMUSTAT & (CFM_ACCERR | CFM_PVIOL))  {
      CFM_CFMUSTAT |= (CFM_ACCERR | CFM_PVIOL);
      return (1);                     // Return fail
    }

    while (!(CFM_CFMUSTAT & CFM_CBEIF));// Wait for command buffer empty
  }

  CFM_CFMSEC |= CFM_SEC(2);             // Unsecure Flash memory

  while (!(CFM_CFMUSTAT & CFM_CCIF  )); // Wait for command to finish

  return (0);                           // Done successfully
}
