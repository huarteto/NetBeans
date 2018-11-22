/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for ST Microelectronics STR91x Flash                  */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"                 // FlashOS Structures

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

#define STACK_SIZE      64              // Stack Size


#ifdef STR91xFxx2
#define BANK0_SIZE      0x00040000      // Bank0 Size = 256kB
#define BANK0_SZ        3               // Bank0 Size Selector
#define BANK0_SEC_CNT   4               // Bank0 Sector Count
#define BANK0_SEC_MASK  0x0000000F      // Bank0 Sector Mask
#define BANK0_SEC_SZ    0x10000         // Bank0 Sector Size
#define BANK0_PL2_OFS   0x10            // Bank0 Protection Level 2 Reg Offset
#define BANK1_SIZE      0x00008000      // Bank0 Size = 32kB
#define BANK1_SZ        2               // Bank1 Size Selector
#define BANK1_SEC_CNT   4               // Bank1 Sector Count
#define BANK1_SEC_MASK  0x00000F00      // Bank1 Sector Mask
#define BANK1_SEC_SZ    0x2000          // Bank1 Sector Size
#define BANK1_PL2_OFS   0x10            // Bank1 Protection Level 2 Reg Offset
#endif
#ifdef STR91xFxx4
#define BANK0_SIZE      0x00080000      // Bank0 Size = 512kB
#define BANK0_SZ        4               // Bank0 Size Selector
#define BANK0_SEC_CNT   8               // Bank0 Sector Count
#define BANK0_SEC_MASK  0x000000FF      // Bank0 Sector Mask
#define BANK0_SEC_SZ    0x10000         // Bank0 Sector Size
#define BANK0_PL2_OFS   0x10            // Bank0 Protection Level 2 Reg Offset
#define BANK1_SIZE      0x00008000      // Bank0 Size = 32kB
#define BANK1_SZ        2               // Bank1 Size Selector
#define BANK1_SEC_CNT   4               // Bank1 Sector Count
#define BANK1_SEC_MASK  0x00000F00      // Bank1 Sector Mask
#define BANK1_SEC_SZ    0x2000          // Bank1 Sector Size
#define BANK1_PL2_OFS   0x10            // Bank1 Protection Level 2 Reg Offset
#endif
#ifdef STR91xFxx6
#define BANK0_SIZE      0x00100000      // Bank0 Size = 1MB
#define BANK0_SZ        5               // Bank0 Size Selector
#define BANK0_SEC_CNT   16              // Bank0 Sector Count
#define BANK0_SEC_MASK  0x0000FFFF      // Bank0 Sector Mask
#define BANK0_SEC_SZ    0x10000         // Bank0 Sector Size
#define BANK0_PL2_OFS   0x14            // Bank1 Protection Level 2 Reg Offset
#define BANK1_SIZE      0x00020000      // Bank0 Size = 128kB
#define BANK1_SZ        4               // Bank1 Size Selector
#define BANK1_SEC_CNT   8               // Bank1 Sector Count
#define BANK1_SEC_MASK  0x000000FF      // Bank1 Sector Mask
#define BANK1_SEC_SZ    0x4000          // Bank1 Sector Size
#define BANK1_PL2_OFS   0x18            // Bank1 Protection Level 2 Reg Offset
#endif
#ifdef STR91xFxx7
#define BANK0_SIZE      0x00200000      // Bank0 Size = 2MB
#define BANK0_SZ        6               // Bank0 Size Selector
#define BANK0_SEC_CNT   32              // Bank0 Sector Count
#define BANK0_SEC_MASK  0xFFFFFFFF      // Bank0 Sector Mask
#define BANK0_SEC_SZ    0x10000         // Bank0 Sector Size
#define BANK0_PL2_OFS   0x14            // Bank1 Protection Level 2 Reg Offset
#define BANK1_SIZE      0x00020000      // Bank0 Size = 128kB
#define BANK1_SZ        4               // Bank1 Size Selector
#define BANK1_SEC_CNT   8               // Bank1 Sector Count
#define BANK1_SEC_MASK  0x000000FF      // Bank1 Sector Mask
#define BANK1_SEC_SZ    0x4000          // Bank1 Sector Size
#define BANK1_PL2_OFS   0x18            // Bank1 Protection Level 2 Reg Offset
#endif

#ifdef BANK0
#define BANK_SIZE       BANK0_SIZE      // Bank Size
#define BANK_SEC_CNT    BANK0_SEC_CNT   // Bank Sector Count
#define BANK_SEC_MASK   BANK0_SEC_MASK  // Bank Secotr Mask
#define BANK_SEC_SZ     BANK0_SEC_SZ    // Bank Sector Size
#define BANK_PL2_OFS    BANK0_PL2_OFS   // Bank Protection Level 2 Reg Offset
#endif

#ifdef BANK1
#define BANK_SIZE       BANK1_SIZE      // Bank Size
#define BANK_SEC_CNT    BANK1_SEC_CNT   // Bank Sector Count
#define BANK_SEC_MASK   BANK1_SEC_MASK  // Bank Secotr Mask
#define BANK_SEC_SZ     BANK1_SEC_SZ    // Bank Sector Size
#define BANK_PL2_OFS    BANK1_PL2_OFS   // Bank Protection Level 2 Reg Offset
#endif


// System Control Unit Registers
#define SCU_CLKCNTR     (*((volatile unsigned long  *) 0x5C002000))
#define SCU_SCR0        (*((volatile unsigned long  *) 0x5C002034))

// Flash Memory Interface Registers
#define FMI_BBSR        (*((volatile unsigned long  *) 0x54000000))
#define FMI_BBADR       (*((volatile unsigned long  *) 0x5400000C))
#define FMI_NBBSR       (*((volatile unsigned long  *) 0x54000004))
#define FMI_NBBADR      (*((volatile unsigned long  *) 0x54000010))
#define FMI_CR          (*((volatile unsigned long  *) 0x54000018))
#define FMI_SR          (*((volatile unsigned long  *) 0x5400001C))

// Flash Commands
#define CMD_PROT1CFM    0x01            // Protect Level 1 Confirm
#define CMD_SERS        0x20            // Sector Erase Set-up
#define CMD_PRGS        0x40            // Program Set-up
#define CMD_CLRSTAT     0x50            // Clear Status Register
#define CMD_PROT1S      0x60            // Protect Level 1 Set-up
#define CMD_RDSTAT      0x70            // Read Status Register
#define CMD_BNKERS      0x80            // Bank Erase Set-up
#define CMD_RSIG        0x90            // Read Electronic Signature
#define CMD_RDOTP       0x98            // Read OTP Sector
#define CMD_CFM         0xD0            // Prog/Ers Resume, Ers Confirm
                                        // Level 1 Unprotect Confirm
#define CMD_RDARR       0xFF            // Read Array

// Status register bits
#define PECS            0x80            // Prog/Ers Controller Status
#define ESS             0x40            // Erase Suspend Status
#define ES              0x20            // Erase Status
#define PS              0x10            // Program Status
#define PSS             0x04            // Program Suspend Status
#define SP              0x02            // Sector Protection Status


unsigned long base_adr;


/*
 *  Delay
 */

void Delay (void) {
  int i;

  for (i = 0; i < 10; i++);
}


/*
 *  Wait for last operation with Flash to finish and return status
 *    Parameter:      adr:  Address on which operation is running
 *    Return Value:   status
 */

unsigned char WaitWithStatus (unsigned long adr) {
  unsigned char status;

  while (!(M16(adr) & PECS));           // Wait for operation finish

  status   = M16(adr);                  // Read status

  M16(adr) = CMD_CLRSTAT;               // Clear Status Register
  M16(adr) = CMD_RDARR;                 // exit Read Status Register mode

  return (status);
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {
  unsigned long i, j;
  unsigned long bank1_adr;

  base_adr = adr & ~(BANK_SIZE - 1);    // Align to Size Boundary

  // Setup System Control Unit
  SCU_CLKCNTR = 0x00020002;             // Main Clock Source is Oscillator
  SCU_SCR0   &=~0x00000001;             // PFQBC Unit Disable

  // Setup Flash Memory Interface
  //  Bank0 is Boot Bank
  //  Bank1 is Non-boot Bank
#ifdef BANK0
  bank1_adr   = base_adr + BANK0_SIZE;
  FMI_BBSR    = BANK0_SZ;
  FMI_BBADR   = base_adr >> 2;
  FMI_NBBSR   = BANK1_SZ;
  FMI_NBBADR  = bank1_adr >> 2;
  FMI_CR      = 0x00000018;             // Enable Bank 0 & 1
#endif  
#ifdef BANK1
  bank1_adr   = base_adr;
  FMI_NBBSR   = BANK1_SZ;
  FMI_NBBADR  = bank1_adr >> 2;
  FMI_CR      = 0x00000010;             // Enable Bank 1 Only
#endif  

  // Clear Level 1 Protection (unprotect all sectors)
  for (i = 0, j = base_adr; i < BANK_SEC_CNT; i++, j += BANK_SEC_SZ) {
    M16(j) = CMD_PROT1S;
    M16(j) = CMD_CFM;
    M16(j) = CMD_RDARR;
    Delay();
  }

  // Check if all sectors are unprotected
  M16(bank1_adr)  = CMD_RSIG;
  i = M16(bank1_adr + BANK_PL2_OFS) & BANK_SEC_MASK;
  M16(bank1_adr)  = CMD_RDARR;          // Leave RSIG mode

  if (i)
    return (1);                         // Not unprotected

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

  // Issue Erase Bank procedure
  M16(base_adr) = CMD_BNKERS;
  M16(base_adr) = CMD_CFM;

  if (WaitWithStatus(base_adr) & (ES | SP)) // Erase unsuccessful
    return (1);

  return (0);                           // Done
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  // Issue Erase Sector procedure
  M16(adr) = CMD_SERS;
  M16(adr) = CMD_CFM;

  if (WaitWithStatus(adr) & (ES | SP))  // Erase unsuccessful
    return (1);

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

  for (i = 0; i < ((sz+1)/2); i++)  {
    M16(adr & ~3) = CMD_PRGS;           // Write Program Set-up Command
    M16(adr) = *((unsigned short *)buf);// Write 2 byte data
    if (WaitWithStatus(adr & ~3) & (PS | SP)) // Program unsuccessful
      return (1);
    buf += 2;
    adr += 2;
  }

  return (0);                           // Done successfully
}
