/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2004 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for ADuC702x (16-bit Bus)                             */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#define STACK_SIZE   64        // Stack Size


/*
 *  ADuC702X Flash MMRS
 */

#define FEESTA (*(volatile unsigned long *) 0xFFFFF800)
#define FEEMOD (*(volatile unsigned long *) 0xFFFFF804)
#define FEECON (*(volatile unsigned long *) 0xFFFFF808)
#define FEEDAT (*(volatile unsigned long *) 0xFFFFF80C)
#define FEEADR (*(volatile unsigned long *) 0xFFFFF810)


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  FEEMOD = 0x09;

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
  unsigned long Flash_Status;

  // Mass Erase Block
  FEEMOD = 0x08;
  FEEADR = 0xFFC3;
  FEEDAT = 0x3CFF;
  FEECON = 0x06;
  Flash_Status = FEESTA;                 //  Load Status of Flash
  while ((Flash_Status & 4) == 4)        //  Wait until Flash Command
  {                                      //  is completed
    Flash_Status = FEESTA;
  }
  if ((Flash_Status & 2) == 2)           //  Fail if Fail Bit set
  {
    return (1);                          //  Command Failed
  }
  else
  {
    return (0);                          //  Command Passed
  }
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  unsigned long Flash_Status;

  // Start Erase Sector Command
  FEEADR = adr;                          //  Load Address to erase
  FEECON = 0x05;                         //  Erase Page Command
  Flash_Status = FEESTA;                 //  Load Status of Flash
  while ((Flash_Status & 4) == 4)        //  Wait until Flash Command
  {                                      //  is completed
    Flash_Status = FEESTA;
  }
  if ((Flash_Status & 2) == 2)           //  Fail if Fail Bit set
  {
    return (1);                          //  Command Failed
  }
  else
  {
    return (0);                          //  Command Passed
  }
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long Flash_Status;
  int  i;

  for (i = 0; i < ((sz+1)/2); i++)  {    //  Write (sz+1)/2 times
    // Start Program Command                 We write in half words
    FEEADR = adr + i * 2;                //  Set Address to write too
    FEEDAT = *((unsigned short *) buf);  //  Load Data to write
    FEECON = 0x02;                       //  Execute Write
    buf += 2;                            //  Increment Buffer location by 2
    Flash_Status = FEESTA;
    while ((Flash_Status & 4) == 4)      //  Wait until Flash command is
    {                                    //  completed
      Flash_Status = FEESTA;
    }
    if ((Flash_Status & 2) == 2)         //  Fail if Fail Bit set
    {
      return (1);                        //  Command Failed
    }
  }
  return (0);                            //  Command Passed
}
