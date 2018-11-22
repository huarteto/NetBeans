/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for Philips LPC2xxx 64kB Flash                        */
/*               using Flash Boot Loader with IAP                      */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

// Memory Mapping Control
#define MEMMAP   (*((volatile unsigned char *) 0xE01FC040))

// Phase Locked Loop (PLL)
#define PLLCON   (*((volatile unsigned char *) 0xE01FC080))
#define PLLCFG   (*((volatile unsigned char *) 0xE01FC084))
#define PLLSTAT  (*((volatile unsigned short*) 0xE01FC088))
#define PLLFEED  (*((volatile unsigned char *) 0xE01FC08C))


#define STACK_SIZE     64      // Stack Size

#define SET_VALID_CODE 1       // Set Valid User Code Signature


unsigned long CCLK;            // CCLK in kHz

struct sIAP {                  // IAP Structure
  unsigned long cmd;           // Command
  unsigned long par[4];        // Parameters
  unsigned long stat;          // Status
  unsigned long res;           // Result
} IAP;


/*
 * IAP Execute
 *    Parameter:      pIAP:  Pointer to IAP
 *    Return Value:   None (stat in IAP)
 */

void IAP_Execute (struct sIAP *pIAP);


/*
 * Get Sector Number
 *    Parameter:      adr:  Sector Address
 *    Return Value:   Sector Number
 */

unsigned long GetSecNum (unsigned long adr) {
  unsigned long n;

  n = (adr >> 13) & 0x0F;                      // Sector Number
  return (n);
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

//CCLK /=  1000;                               // Clock Frequency in kHz
  CCLK  = (1049*(clk >> 10)) >> 10;            // Approximate (no Library Code)

  PLLCON  = 0x00;                              // Disable PLL (use Oscillator)
  PLLFEED = 0xAA;                              // Feed Sequence Part #1
  PLLFEED = 0x55;                              // Feed Sequence Part #2

  MEMMAP  = 0x01;                              // User Flash Mode

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

  IAP.cmd    = 50;                             // Prepare Sector for Erase
  IAP.par[0] = 0;                              // Start Sector
  IAP.par[1] = 7;                              // End Sector
  IAP_Execute (&IAP);                          // Execute IAP Command
  if (IAP.stat) return (1);                    // Command Failed

  IAP.cmd    = 52;                             // Erase Sector
  IAP.par[0] = 0;                              // Start Sector
  IAP.par[1] = 7;                              // End Sector
  IAP.par[2] = CCLK;                           // CCLK in kHz
  IAP_Execute (&IAP);                          // Execute IAP Command
  if (IAP.stat) return (1);                    // Command Failed

  return (0);                                  // Finished without Errors
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  unsigned long n;

  n = GetSecNum(adr);                          // Get Sector Number

  IAP.cmd    = 50;                             // Prepare Sector for Erase
  IAP.par[0] = n;                              // Start Sector
  IAP.par[1] = n;                              // End Sector
  IAP_Execute (&IAP);                          // Execute IAP Command
  if (IAP.stat) return (1);                    // Command Failed

  IAP.cmd    = 52;                             // Erase Sector
  IAP.par[0] = n;                              // Start Sector
  IAP.par[1] = n;                              // End Sector
  IAP.par[2] = CCLK;                           // CCLK in kHz
  IAP_Execute (&IAP);                          // Execute IAP Command
  if (IAP.stat) return (1);                    // Command Failed

  return (0);                                  // Finished without Errors
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {
  unsigned long n;

#if SET_VALID_CODE != 0                        // Set valid User Code Signature
  if (adr == 0) {                              // Check for Interrupt Vectors
    n = *((unsigned long *)(buf + 0x00)) +     // Reset Vector
        *((unsigned long *)(buf + 0x04)) +     // Undefined Instruction Vector
        *((unsigned long *)(buf + 0x08)) +     // Software Interrupt Vector
        *((unsigned long *)(buf + 0x0C)) +     // Prefetch Abort Vector
        *((unsigned long *)(buf + 0x10)) +     // Data Abort Vector
        *((unsigned long *)(buf + 0x18)) +     // IRQ Vector
        *((unsigned long *)(buf + 0x1C));      // FIQ Vector
    *((unsigned long *)(buf + 0x14)) = 0 - n;  // Signature at Reserved Vector
  }
#endif

  n = GetSecNum(adr);                          // Get Sector Number

  IAP.cmd    = 50;                             // Prepare Sector for Write
  IAP.par[0] = n;                              // Start Sector
  IAP.par[1] = n;                              // End Sector
  IAP_Execute (&IAP);                          // Execute IAP Command
  if (IAP.stat) return (1);                    // Command Failed

  IAP.cmd    = 51;                             // Copy RAM to Flash
  IAP.par[0] = adr;                            // Destination Flash Address
  IAP.par[1] = (unsigned long)buf;             // Source RAM Address
  IAP.par[2] = 1024;                           // Fixed Page Size
  IAP.par[3] = CCLK;                           // CCLK in kHz
  IAP_Execute (&IAP);                          // Execute IAP Command
  if (IAP.stat) return (1);                    // Command Failed

  return (0);                                  // Finished without Errors
}
