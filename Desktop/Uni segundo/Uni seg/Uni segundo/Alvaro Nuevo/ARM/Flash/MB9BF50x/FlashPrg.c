/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2006-2010 Keil - An ARM Company.                     */
/*  All rights reserved.                                               */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Flash Programming Functions adapted for               */
/*               Fujitsu MB9BF50x Flash                                */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

typedef volatile unsigned long    vu32;
typedef          unsigned long     u32;
typedef volatile unsigned short   vu16;
typedef          unsigned short    u16;

#define M16(adr) (*((vu16 *) (adr)))

// Peripheral Memory Map
#define FIF_BASE          (0x40000000)
#define HWWDT_BASE        (0x40011000)

#define FIF               ((FIF_TypeDef *) FIF_BASE)
#define HWWDT             ((WDT_TypeDef *) HWWDT_BASE)


typedef struct {             // Flash Interface
  vu32 FASZR;                // Offset: 0x000 (R/W) Flash Access Size Register
  vu32 FRWTR;                // Offset: 0x004 (R/W) Flash Read Wait Register
  vu32 FSTR;                 // Offset: 0x008 (R/W) Flash Status Register
  vu32 RESERVED0;
  vu32 FSYNDN;               // Offset: 0x010 (R/W) Flash Sync Down Register
} FIF_TypeDef;

typedef struct {             // Watchdog Timer
  vu32 LDR;                  // offset: 0x000 (R/W) Watchdog Timer Load Register
  vu32 VLR;                  // Offset: 0x004 (R/W) Watchdog Timer Value Register
  vu32 CTL;                  // Offset: 0x008 (R/W) Watchdog Timer Control Register
  vu32 ICL;                  // Offset: 0x00C (R/W) Watchdog Timer Clear Register
  vu32 RIS;                  // Offset: 0x010 (R/W) Watchdog Timer Interrupt Status Register 
  vu32 RESERVED0[763];
  vu32 LCK;                  // Offset: 0xC00 (R/W) Watchdog Timer Lock Register
} WDT_TypeDef;


/*
  -------------+-----------+-------------+-------------+-------------+-------------+-------------+--------------
               |           |  1st write  |  2nd write  |  3rd write  |  4th write  |  5th write  |  6th write
     Command   | Number of |-------------+-------------+-------------+-------------+-------------+--------------
               | writes    | Addr.| Data | Addr.| Data | Addr.| Data | Addr.| Data | Addr.| Data | Addr.| Data 
  -------------+-----------+------+------+------+------+------+------+------+------+------+------+------+-------
  Read/Reset         1      0xXXX  0xF0   RA     RD     --     --     --     --     --     --     --     --
  Read/Reset         4      0xAA8  0xAA   0x554  0x55   0xAA8  0xF0   RA     RD     --     --     --     --
  Write              4      0xAA8  0xAA   0x554  0x55   0xAA8  0xA0   PA     PD     --     --     --     --
  Chip erase         6      0xAA8  0xAA   0x554  0x55   0xAA8  0x80   0xAA8  0xAA   0x554  0x55   0xAA8  0x10
  Sector erase       6      0xAA8  0xAA   0x554  0x55   0xAA8  0x80   0xAA8  0xAA   0x554  0x55   SA     0x30
 */

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


/*
 * Check if Program/Erase completed
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int Polling (unsigned long adr) {
  unsigned int q6;

  fsr.v = M16(adr);
  q6 = fsr.b.q6;
  do {
    fsr.v = M16(adr);
    if (fsr.b.q6 == q6) return (0);  // Done
    q6 = fsr.b.q6;
  } while (fsr.b.q5 == 0);           // Check for Timeout
  fsr.v = M16(adr);
  q6 = fsr.b.q6;
  fsr.v = M16(adr);
  if (fsr.b.q6 == q6) return (0);    // Done
  M16(adr) = 0xF0;                   // Reset Device
  return (1);                        // Failed
}


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  HWWDT->LCK = 0x1ACCE551;
  HWWDT->LCK = 0xE5331AAE;
  HWWDT->CTL = 0;                                       // Disable Watchdog

  base_adr = adr;

  return (0);                                           // Done
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {

  return (0);                                           // Done
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {
  vu32 dummy;

  FIF->FASZR = 0x01;                                    // set CPU programming mode
  dummy = FIF->FASZR;                                   // dummy read

  // Start Chip Erase Command
  M16(base_adr + 0xAA8) = 0xAA;
  M16(base_adr + 0x554) = 0x55;
  M16(base_adr + 0xAA8) = 0x80;
  M16(base_adr + 0xAA8) = 0xAA;
  M16(base_adr + 0x554) = 0x55;
  M16(base_adr + 0xAA8) = 0x10;

  if (Polling(base_adr) != 0) return (1);               // Wait until Erase completed

  FIF->FASZR = 0x02;                                    // set CPU ROM mode
  dummy = FIF->FASZR;                                   // dummy read

  return (0);
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {
  vu32 dummy;

  FIF->FASZR = 0x01;                                    // set CPU programming mode
  dummy = FIF->FASZR;                                   // dummy read

  // Start Erase Sector Command
  M16(base_adr + 0xAA8) = 0xAA;
  M16(base_adr + 0x554) = 0x55;
  M16(base_adr + 0xAA8) = 0x80;
  M16(base_adr + 0xAA8) = 0xAA;
  M16(base_adr + 0x554) = 0x55;
  M16(adr) = 0x30;

  do {
    fsr.v = M16(adr);
  } while (fsr.b.q3 == 0);                              // Wait for Sector Erase Timeout

  if (Polling(adr) != 0) return (1);                    // Wait until Erase completed

  FIF->FASZR = 0x02;                                    // set CPU ROM mode
  dummy = FIF->FASZR;                                   // dummy read

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
  int  i;
  vu32 dummy;

  FIF->FASZR = 0x01;                                    // set CPU programming mode
  dummy = FIF->FASZR;                                   // dummy read

  for (i = 0; i < ((sz+1)/2); i++)  {
    // Start Program Command
    M16(base_adr + 0xAA8) = 0xAA;
    M16(base_adr + 0x554) = 0x55;
    M16(base_adr + 0xAA8) = 0xA0;
    M16(adr) = *((u16 *) buf);
    if (Polling(adr) != 0) return (1);
    buf += 2;
    adr += 2;
  }

  FIF->FASZR = 0x02;                                    // set CPU ROM mode
  dummy = FIF->FASZR;                                   // dummy read

  return (0);
}
