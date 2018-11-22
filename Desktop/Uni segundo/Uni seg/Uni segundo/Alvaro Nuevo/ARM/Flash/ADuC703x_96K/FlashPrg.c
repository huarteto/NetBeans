/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2004 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for ADuC703x                                          */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#define STACK_SIZE   64        // Stack Size


/*
 *  ADuC703X Flash MMRS
 */

#define FEE0STA (*(volatile unsigned long *) 0xFFFF0E00)
#define FEE0MOD (*(volatile unsigned long *) 0xFFFF0E04)
#define FEE0CON (*(volatile unsigned long *) 0xFFFF0E08)
#define FEE0DAT (*(volatile unsigned long *) 0xFFFF0E0C)
#define FEE0ADR (*(volatile unsigned long *) 0xFFFF0E10)

#define FEE1STA (*(volatile unsigned long *) 0xFFFF0E80)
#define FEE1MOD (*(volatile unsigned long *) 0xFFFF0E84)
#define FEE1CON (*(volatile unsigned long *) 0xFFFF0E88)
#define FEE1DAT (*(volatile unsigned long *) 0xFFFF0E8C)
#define FEE1ADR (*(volatile unsigned long *) 0xFFFF0E90)

#define T3CLRI  (*(volatile unsigned long *) 0xFFFF036C)


int Blk1Exist (void);

int EraseSectorBlk0 (unsigned long adr);
int EraseSectorBlk1 (unsigned long adr);

int ProgramPageBlk0 (unsigned long adr, unsigned long sz, unsigned char *buf);
int ProgramPageBlk1 (unsigned long adr, unsigned long sz, unsigned char *buf);

int FlashCfg = 0;


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  FEE0MOD = 0x08;
  FEE1MOD = 0x08;
  T3CLRI  = 0x1;    // Refresh Watchdog

  // Returns 1 if the block 1 exists, 0 otherwise
  FlashCfg = 1;

  return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  T3CLRI  = 0x1;    // Refresh Watchdog
  FEE0MOD = 0x00;
  FEE1MOD = 0x00;
  return (0);
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {
  unsigned long Flash_Status;

  // User Mass Erase of both Blocks
  T3CLRI  = 0x1;    // Refresh Watchdog
  FEE0MOD = 0x08;
  FEE0ADR = 0xFFC3;
  FEE0DAT = 0x3CFF;
  FEE0CON = 0x06;
  Flash_Status = FEE0STA;                // Load Status of Flash

  while ((Flash_Status & 4) == 4)        // Wait until Flash Command
  {
    T3CLRI  = 0x1;                       // Refresh Watchdog
    Flash_Status = FEE0STA;              // is completed
  }

  if ((Flash_Status & 2) == 2)           // Fail if Fail Bit set
    return(1);                           // Command Failed
  else
  {
    if (FlashCfg == 1)
    {
      FEE1MOD = 0x08;
      FEE1ADR = 0xFFC3;
      FEE1DAT = 0x3CFF;
      FEE1CON = 0x06;
      Flash_Status = FEE1STA;            // Load Status of Flash

      while ((Flash_Status & 4) == 4)    // Wait until Flash Command
      {
        T3CLRI       = 0x1;              // Refresh Watchdog
        Flash_Status = FEE1STA;          // is completed
      }

      if ((Flash_Status & 2) == 2)       // Fail if Fail Bit set
        return(1);                       // Command Failed
      else
        return(0);                       // Command passed
    }
    else
      return(0);                         // Command passed
  }
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  int result = 0;

  if (FlashCfg == 0)
    result = EraseSectorBlk0(adr);
  else if (FlashCfg == 1)
  {
    if ((adr & 0x1FFFF) >= 0x10000)
      result = EraseSectorBlk0(adr);
    else
      result = EraseSectorBlk1(adr);
  }
  return result;
}

int EraseSectorBlk0 (unsigned long adr) {
  unsigned long Flash_Status;

  // Start Erase Sector Command
  FEE0ADR = adr;                         // Load Address to erase
  FEE0CON = 0x05;                        // Erase Page Command
  Flash_Status = FEE0STA;                // Load Status of Flash
  while ((Flash_Status & 4) == 4)        // Wait until Flash Command
  {                                      // is completed
    Flash_Status = FEE0STA;
    T3CLRI       = 0x1;                  // Refresh Watchdog
  }
  if ((Flash_Status & 2) == 2)           // Fail if Fail Bit set
  {
    return (1);                          // Command Failed
  }
  else
  {
    return (0);                          // Command Passed
  }
}

int EraseSectorBlk1 (unsigned long adr) {
  unsigned long Flash_Status;

  // Start Erase Sector Command
  FEE1ADR = adr;                         // Load Address to erase
  FEE1CON = 0x05;                        // Erase Page Command
  Flash_Status = FEE1STA;                // Load Status of Flash
  while ((Flash_Status & 4) == 4)        // Wait until Flash Command
  {                                      // is completed
    Flash_Status = FEE1STA;
    T3CLRI       = 0x1;                  // Refresh Watchdog
  }
  if ((Flash_Status & 2) == 2)           // Fail if Fail Bit set
  {
    return (1);                          // Command Failed
  }
  else
  {
    return (0);                          // Command Passed
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
  int result = 0;

  if (FlashCfg == 0)
    result = ProgramPageBlk0(adr, sz, buf);
  else if (FlashCfg == 1)
  {
    if ((adr & 0x1FFFF) >= 0x10000)
      result = ProgramPageBlk0(adr, sz, buf);
    else
      result = ProgramPageBlk1(adr, sz, buf);
  }
  return result;
}

int ProgramPageBlk0 (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long Flash_Status;
  int  i;

  for (i = 0; i < ((sz+1)/2); i++)  {    // Write (sz+1)/2 times
    // Start Program Command                We write in half words
    T3CLRI  = 0x1;                       // Refresh Watchdog
    FEE0ADR = adr + i * 2;               // Set Address to write too
    FEE0DAT = *((unsigned short *) buf); // Load Data to write
    FEE0CON = 0x02;                      // Execute Write
    buf += 2;                            // Increment Buffer location by 2
    Flash_Status = FEE0STA;
    while ((Flash_Status & 4) == 4)      // Wait until Flash command is
    {
      T3CLRI       = 0x1;                // Refresh Watchdog
      Flash_Status = FEE0STA;
    }
    if ((Flash_Status & 2) == 2)         // Fail if Fail Bit set
    {
      return (1);                        // Command Failed
    }
  }
  return (0);                            // Command Passed
}

int ProgramPageBlk1 (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long Flash_Status;
  int  i;

  for (i = 0; i < ((sz+1)/2); i++)  {    // Write (sz+1)/2 times
    // Start Program Command                We write in half words
    T3CLRI  = 0x1;                       // Refresh Watchdog
    FEE1ADR = adr + i * 2;               // Set Address to write too
    FEE1DAT = *((unsigned short *) buf); // Load Data to write
    FEE1CON = 0x02;                      // Execute Write
    buf += 2;                            // Increment Buffer location by 2
    Flash_Status = FEE1STA;
    while ((Flash_Status & 4) == 4)      // Wait until Flash command is
    {
      T3CLRI       = 0x1;                // Refresh Watchdog
      Flash_Status = FEE1STA;
    }
    if ((Flash_Status & 2) == 2)         // Fail if Fail Bit set
    {
      return (1);                        // Command Failed
    }
  }
  return (0);                            // Command Passed
}
