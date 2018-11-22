/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2004 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for ADuC712x (16-bit Bus)                             */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#define STACK_SIZE   64        // Stack Size


/*
 *  ADuC712x Flash MMRS
 */

/* Flash Control Block 0 Registers                     */
#define FEE0STA (*(volatile unsigned short *) 0xFFFF0E00)
#define FEE0MOD (*(volatile unsigned short *) 0xFFFF0E04)
#define FEE0CON (*(volatile unsigned short *) 0xFFFF0E08)
#define FEE0DAT (*(volatile unsigned short *) 0xFFFF0E0C)
#define FEE0ADR (*(volatile unsigned short *) 0xFFFF0E10)

/* Flash Control Block 1 Registers                     */
#define FEE1STA (*(volatile unsigned short *) 0xFFFF0E80)
#define FEE1MOD (*(volatile unsigned short *) 0xFFFF0E84)
#define FEE1CON (*(volatile unsigned short *) 0xFFFF0E88)
#define FEE1DAT (*(volatile unsigned short *) 0xFFFF0E8C)
#define FEE1ADR (*(volatile unsigned short *) 0xFFFF0E90)


int IsBlk0Addr       (unsigned long  adr);
int EraseSectorBlk0  (unsigned short adr);
int EraseSectorBlk1  (unsigned short adr);
int ProgramPageBlk0  (unsigned short adr, unsigned long sz, unsigned char *buf);
int ProgramPageBlk1  (unsigned short adr, unsigned long sz, unsigned char *buf);

#define FLASH_MASK   0x1FFFF
#define BLOCK_MASK   0x0FFFF
#define _64K         0x10000


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc)
{
  FEE0MOD = 0x09;
  FEE1MOD = 0x09;
  return(0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc)
{
  FEE0MOD = 0x00;
  FEE1MOD = 0x00;
  return(0);
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void)
{
  unsigned long Flash_Status;

  // User Mass Erase of both Blocks
  FEE0MOD = 0x08;
  FEE0ADR = 0xFFC3;
  FEE0DAT = 0x3CFF;
  FEE0CON = 0x06;
  Flash_Status = FEE0STA;                //  Load Status of Flash
  while ((Flash_Status & 4) == 4)        //  Wait until Flash Command
    Flash_Status = FEE0STA;              //  is completed

  if ((Flash_Status & 2) == 2)           //  Fail if Fail Bit set
    return(1);                           //  Command Failed
  else
  {
    FEE1MOD  = 0x08;
    FEE1ADR  = 0xFFC3;
    FEE1DAT  = 0x3CFF;
    FEE1CON  = 0x06;
    Flash_Status = FEE1STA;              //  Load Status of Flash

    while ((Flash_Status & 4) == 4)      //  Wait until Flash Command
      Flash_Status = FEE1STA;            //  is completed

    if ((Flash_Status & 2) == 2)         //  Fail if Fail Bit set
       return(1);                        //  Command Failed
    else
       return(0);                        //  Command passed
  }
}


/*
 *  Check if address is in block 0
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - No,  1 - Yes
 *
 *  The ADuc712x can only be configured for 128K
 *  or 96K so both blocks will be present always.
 *
 *  In all ADI sillicon, block 0 is always the highest
 *  block. => >= 64K implies block 0
 */

int IsBlk0Addr (unsigned long adr)
{
  if ((adr & FLASH_MASK) >= _64K)
    return (1);
  else
    return (0);
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr)
{
  int result;

  if (IsBlk0Addr(adr))
    result = EraseSectorBlk0(adr & BLOCK_MASK);
  else
    result = EraseSectorBlk1(adr & BLOCK_MASK);
  return result;
}

int EraseSectorBlk0(unsigned short adr)
{
  unsigned long Flash_Status;

  // Start Erase Sector Command
  FEE0ADR      = adr;                    //  Load Address to erase
  FEE0CON      = 0x05;                   //  Erase Page Command
  Flash_Status = FEE0STA;                //  Load Status of Flash

  while ((Flash_Status & 4) == 4)        //  Wait until Flash Command
    Flash_Status = FEE0STA;              //  is completed

  if ((Flash_Status & 2) == 2)           //  Fail if Fail Bit set
    return(1);                           //  Command Failed
  else
    return(0);                           //  Command Passed
}

int EraseSectorBlk1(unsigned short adr)
{

  unsigned long Flash_Status;

  // Start Erase Sector Command
  FEE1ADR      = adr;                    //  Load Address to erase
  FEE1CON      = 0x05;                   //  Erase Page Command
  Flash_Status = FEE1STA;                //  Load Status of Flash

  while ((Flash_Status & 4) == 4)        //  Wait until Flash Command
    Flash_Status = FEE1STA;              //  is completed

  if ((Flash_Status & 2) == 2)           //  Fail if Fail Bit set
    return(1);                           //  Command Failed
  else
    return(0);                           //  Command Passed
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */
int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf)
{
  int result;

  if (IsBlk0Addr(adr))
    result = ProgramPageBlk0(adr,sz,buf);
  else
    result = ProgramPageBlk1(adr,sz,buf);

  return result;
}

int ProgramPageBlk0 (unsigned short adr, unsigned long sz, unsigned char *buf)
{

  unsigned long Flash_Status;
  int  i;

  for (i = 0; i < ((sz+1)/2); i++)
  {                                      //  Write (sz+1)/2 times
    // Start Program Command                 We write in half words
    FEE0ADR = adr + i * 2;               //  Set Address to write too
    FEE0DAT = *((unsigned short *) buf); //  Load Data to write
    FEE0CON = 0x02;                      //  Execute Write
    buf += 2;                            //  Increment Buffer location by 2
    Flash_Status = FEE0STA;
    while ((Flash_Status & 4) == 4)      //  Wait until Flash command is
    {                                    //  completed
       Flash_Status = FEE0STA;
    }
    if ((Flash_Status & 2) == 2)         //  Fail if Fail Bit set
    {
      return(1);                         //  Command Failed
    }
  }
  return(0);                             //  Command Passed
}

int ProgramPageBlk1 (unsigned short adr, unsigned long sz, unsigned char *buf)
{

  unsigned long Flash_Status;
  int  i;

  for (i = 0; i < ((sz+1)/2); i++)
  {                                      //  Write (sz+1)/2 times
    // Start Program Command                 We write in half words
    FEE1ADR = adr + i * 2;               //  Set Address to write too
    FEE1DAT = *((unsigned short *) buf); //  Load Data to write
    FEE1CON = 0x02;                      //  Execute Write
    buf += 2;                            //  Increment Buffer location by 2
    Flash_Status = FEE1STA;
    while ((Flash_Status & 4) == 4)      //  Wait until Flash command is
    {                                    //  completed
      Flash_Status = FEE1STA;
    }
    if ((Flash_Status & 2) == 2)         //  Fail if Fail Bit set
    {
      return(1);                         //  Command Failed
    }
  }
  return(0);                             //  Command Passed
}
