/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2006                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted                   */
/*               for SST39x320x (16-bit Bus) on BU1511KV2 Board        */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

#define M8(adr)  (*((volatile unsigned char  *) (adr)))
#define M16(adr) (*((volatile unsigned short *) (adr)))
#define M32(adr) (*((volatile unsigned long  *) (adr)))

// System Control WPB
#define SYSCTRL_WPB     (*((volatile unsigned char *) 0xC0006040))

#define STACK_SIZE   64        // Stack Size


union fsreg {                  // Flash Status Register
  struct b  {
    unsigned int q0:1;
    unsigned int q1:1;
    unsigned int q2:1;
    unsigned int q3:1;
    unsigned int q4:1;
    unsigned int q5:1;
    unsigned int q6:1;
    unsigned int q7:1;
  } b;
  unsigned int v;
} fsr;

unsigned long base_adr;
unsigned long CP15_ControlReg;
unsigned char SysCtrl_WPB;


/*
 * Disable DATA and I_TCM cach via CP15 Control Register
 *    Parameter:      none
 *    Return Value:   original CP15 Control Register content
 */

__asm unsigned long disable_TCM_Cache(void)
{
  MRC p15, 0, R0, c1, c0, 0                   ; read CP15 Control Register
  BIC R1, R0, #(1<<18):OR:(1<<16)             ; clear bit 18, 16; Instruction TCM and Data Cache disable
  MCR p15, 0, R1, c1, c0, 0                   ; write CP15 Control Register; Disable TCM and Data cache              
  BX LR
}

/*
 * Restore CP15 Control Register
 *    Parameter:      CP15 Control Register content
 *    Return Value:   none
 */

__asm void restore_TCM_Cache(unsigned long CP15_ControlReg)
{
  MCR p15, 0, R0, c1, c0, 0                   ; write CP15 Control Register; restore settings              
  BX LR
}

/*
 * Check if Program/Erase completed
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int Polling (unsigned long adr) {
  unsigned int q6;

  // Check Toggle Bit
  do {
    fsr.v = M16(adr);
    q6 = fsr.b.q6;
    fsr.v = M16(adr);
  } while (fsr.b.q6 != q6);
  return (0);                  // Done
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

  SysCtrl_WPB     = SYSCTRL_WPB;              // SysCtrl_WPB settings
  SYSCTRL_WPB    |= (1 << 4);                 // set EXT_WEB active level
  CP15_ControlReg =  disable_TCM_Cache();

  return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  SYSCTRL_WPB    &= ~(1 << 4);                // reset EXT_WEB active level
  restore_TCM_Cache(CP15_ControlReg);
 
 return (0);
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {

  // Start Chip Erase Command
  M16(base_adr + (0x5555 << 1)) = 0xAA;
  M16(base_adr + (0x2AAA << 1)) = 0x55;
  M16(base_adr + (0x5555 << 1)) = 0x80;
  M16(base_adr + (0x5555 << 1)) = 0xAA;
  M16(base_adr + (0x2AAA << 1)) = 0x55;
  M16(base_adr + (0x5555 << 1)) = 0x10;

  return (Polling(base_adr));  // Wait until Erase completed
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  // Start Erase Sector Command
  M16(base_adr + (0x5555 << 1)) = 0xAA;
  M16(base_adr + (0x2AAA << 1)) = 0x55;
  M16(base_adr + (0x5555 << 1)) = 0x80;
  M16(base_adr + (0x5555 << 1)) = 0xAA;
  M16(base_adr + (0x2AAA << 1)) = 0x55;
  M16(adr) = 0x30;

  return (Polling(adr));       // Wait until Erase completed
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

  for (i = 0; i < ((sz+1)/2); i++)  {
    // Start Program Command
    M16(base_adr + (0x5555 << 1)) = 0xAA;
    M16(base_adr + (0x2AAA << 1)) = 0x55;
    M16(base_adr + (0x5555 << 1)) = 0xA0;
    M16(adr) = *((unsigned short *) buf);
    if (Polling(adr) != 0) return (1);
    buf += 2;
    adr += 2;
  }
  return (0);
}
