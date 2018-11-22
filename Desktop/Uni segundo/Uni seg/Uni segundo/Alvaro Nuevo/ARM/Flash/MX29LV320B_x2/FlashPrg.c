/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2009                                */
/******************************************************************************/
/*                                                                            */
/*  FlashPrg.C:  Flash Programming Functions for                              */
/*               Dual Mxic MX29LV320B NOR Flash Memory (16-bit Bus)           */
/*                                                                            */
/******************************************************************************/

#include "..\FlashOS.H"                 /* FlashOS Structures                 */

#define MEM          M16
#define M8(adr)      (*((volatile unsigned char  *) (adr)))
#define M16(adr)     (*((volatile unsigned short *) (adr)))
#define M32(adr)     (*((volatile unsigned long  *) (adr)))

#define STACK_SIZE   64                 /* Stack Size                         */

#define ADR_SHIFT    1                  /* Number of bit shift for address    */
#define BUS_WIDTH    16                 /* Bus width in bits                  */
#define CMD_RST      (0x0000F0F0)       /* Reset Command                      */
#define CMD_ERS      (0x00008080)       /* Erase Command                      */
#define START_CE     (0x00001010)       /* Start Chip Erase Command           */
#define START_SE     (0x00003030)       /* Start Sector Erase Command         */
#define START_LD     (0x0000A0A0)       /* Start Load Data for Programming    */
#define VAL_AA       (0x0000AAAA)       /* Hex Value 0xAA                     */
#define VAL_55       (0x00005555)       /* Hex Value 0x55                     */
#define DQ7          (0x00008080)       /* DQ7 bit mask                       */
#define DQ6          (0x00004040)       /* DQ6 bit mask                       */
#define DQ5          (0x00002020)       /* DQ5 bit mask                       */
#define DQ3          (0x00000808)       /* DQ3 bit mask                       */
#define DQ2          (0x00000404)       /* DQ2 bit mask                       */
#define DQ1          (0x00000202)       /* DQ1 bit mask                       */

unsigned long base_adr;


/*
 * Check if Program/Erase completed
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int ToggleAlgo (unsigned long adr) {
  unsigned int status, status_ex;

  status_ex = MEM(adr);
  do {
    status  = MEM(adr);
    if (!((status ^ status_ex) & DQ6)) {
      return (0);                       /* Done                               */
    }
    status_ex = status;
  } while (status ^ DQ5);               /* Check for Timeout                  */
  status_ex = MEM(adr);
  status    = MEM(adr);
  if (!((status ^ status_ex) & DQ6)) {
    return (0);                         /* Done                               */
  }
  MEM(adr) = CMD_RST;                   /* Reset Devices                      */
  return (1);                           /* Failed                             */
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

  /* Start Chip Erase Command */
  MEM(base_adr + (0x555 << ADR_SHIFT)) = VAL_AA;
  MEM(base_adr + (0x2AA << ADR_SHIFT)) = VAL_55;
  MEM(base_adr + (0x555 << ADR_SHIFT)) = CMD_ERS;
  MEM(base_adr + (0x555 << ADR_SHIFT)) = VAL_AA;
  MEM(base_adr + (0x2AA << ADR_SHIFT)) = VAL_55;
  MEM(base_adr + (0x555 << ADR_SHIFT)) = START_CE;

  return (ToggleAlgo(base_adr));        /* Wait until Erase completed         */
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  /* Start Erase Sector Command */
  MEM(base_adr + (0x555 << ADR_SHIFT)) = VAL_AA;
  MEM(base_adr + (0x2AA << ADR_SHIFT)) = VAL_55;
  MEM(base_adr + (0x555 << ADR_SHIFT)) = CMD_ERS;
  MEM(base_adr + (0x555 << ADR_SHIFT)) = VAL_AA;
  MEM(base_adr + (0x2AA << ADR_SHIFT)) = VAL_55;
  MEM(adr)                             = START_SE;

  while ((MEM(adr) ^ DQ3) & DQ3);       /* Wait Erase time Window to expire   */

  return (ToggleAlgo(adr));             /* Wait until Erase completed         */
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

  for (i = 0; i < ((sz+1)/(BUS_WIDTH>>3)); i++)  {
    /* Start Program Command */
    MEM(base_adr + (0x555 << ADR_SHIFT)) = VAL_AA;
    MEM(base_adr + (0x2AA << ADR_SHIFT)) = VAL_55;
    MEM(base_adr + (0x555 << ADR_SHIFT)) = START_LD;
    MEM(adr)                             = MEM(buf);
    if (ToggleAlgo(adr)) return (1);    /* Wait until Programming completed   */
    buf += (BUS_WIDTH>>3);
    adr += (BUS_WIDTH>>3);
  }
  return (0);                           /* Done                               */
}
