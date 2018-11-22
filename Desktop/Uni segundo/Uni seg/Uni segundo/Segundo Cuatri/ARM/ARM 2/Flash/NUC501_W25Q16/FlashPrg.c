/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2006-2010 Keil - An ARM Company.                     */
/*  All rights reserved.                                               */
/***********************************************************************/
/*                                                                     */
/*  FlashPrg.c:  Device Description for External SPI Flash             */
/*               W25Q16 programming with Nuvoton NUC501 Devices        */
/*                                                                     */
/***********************************************************************/

#include "..\FlashOS.H"        // FlashOS Structures

typedef volatile unsigned long    vu32;
typedef          unsigned long     u32;
typedef volatile unsigned char    vu8;
typedef          unsigned char     u8;

#define M32(adr) (*((vu32 *) (adr)))

// Peripheral Memory Map
#define CLK_BASE          (0xB1000200)

#define CLK               ((CLK_TypeDef  *) CLK_BASE)


// Clock Control Registers
typedef struct {
  vu32 PWRCON;               // Offset: 0x000 (R/W) System Power Down Control Register
  vu32 AHBCLK;               // Offset: 0x004 (R/W) AHB Device Clock Enable Control Register
  vu32 APBCLK;               // Offset: 0x008 (R/W) APB Device Clock Enable Control Register
  vu32 RESERVED0[1];
  vu32 CLKSEL;               // Offset: 0x010 (R/W) Clock Source Select Control Register
  vu32 CLKDIV0;              // Offset: 0x014 (R/W) Clock Divider Number Register 0
  vu32 CLKDIV1;              // Offset: 0x018 (R/W) Clock Divider Number Register 1
  vu32 RESERVED1[1];
  vu32 MPLLCON;              // Offset: 0x020 (R/W) MPLL Control Register
} CLK_TypeDef;



extern void SPI_FLASH_DMMEnable  (void);
extern void SPI_FLASH_Init       (void);
extern void SPI_FLASH_EraseChip  (void);
extern void SPI_FLASH_EraseSector(u32 addr);
extern void SPI_FLASH_PageWrite  (u8* pBuf, u32 addr, u32 sz);
extern void SPI_FLASH_BufferRead (u8* pBuf, u32 addr, u32 sz);

unsigned char aux_buf[256];             // see FlashDev: Programming Page Size
unsigned long base_adr;                 // Base Address


/*
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Device Base Address
 *                    clk:  Clock Frequency (Hz)
 *                    fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int Init (unsigned long adr, unsigned long clk, unsigned long fnc) {

  switch (fnc)  {
    case 1:                             // Erase   Init
      base_adr = adr;

    case 2:                             // Program Init
    case 3:                             // Verify  Init
      CLK->MPLLCON = 0x0234;            // MPLL = 216MHz @ 12 MHz XTAL
      CLK->CLKSEL  = 0x0002;            // HCLK = MPLL / 2	

      SPI_FLASH_Init();
      break;
  }

  return (0);
}


/*
 *  De-Initialize Flash Programming Functions
 *    Parameter:      fnc:  Function Code (1 - Erase, 2 - Program, 3 - Verify)
 *    Return Value:   0 - OK,  1 - Failed
 */

int UnInit (unsigned long fnc) {
  switch (fnc)  {
    case 1:                             // Erase   Init
    case 2:                             // Program Init
    case 3:                             // Verify  Init
      CLK->MPLLCON = 0x0234;            // MPLL = 216MHz @ 12 MHz XTAL
      CLK->CLKSEL  = 0x0002;            // HCLK = MPLL / 2	

      SPI_FLASH_Init();

//      SPI_FLASH_DMMEnable();
      break;
  }

  return (0);
}


/*
 *  Erase complete Flash Memory
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseChip (void) {

  SPI_FLASH_EraseChip();

  return (0);                           // Done
}


/*
 *  Erase Sector in Flash Memory
 *    Parameter:      adr:  Sector Address
 *    Return Value:   0 - OK,  1 - Failed
 */

int EraseSector (unsigned long adr) {

  SPI_FLASH_EraseSector(adr-base_adr);

  return (0);                           // Done
}


/*
 *  Blank Check Checks if Memory is Blank
 *    Parameter:      adr:  Block Start Address
 *                    sz:   Block Size (in bytes)
 *                    pat:  Block Pattern
 *    Return Value:   0 - OK,  1 - Failed
 */

int BlankCheck (unsigned long adr, unsigned long sz, unsigned char pat) {

  return (1);                           // Always Force Erase
}


/*
 *  Program Page in Flash Memory
 *    Parameter:      adr:  Page Start Address
 *                    sz:   Page Size
 *                    buf:  Page Data
 *    Return Value:   0 - OK,  1 - Failed
 */

int ProgramPage (unsigned long adr, unsigned long sz, unsigned char *buf) {

  SPI_FLASH_PageWrite(buf, adr-base_adr, sz);

  return (0);                           // Done
}


/*
 *
 *  Verify Flash Contents
 *    Parameter:      adr:  Start Address
 *                    sz:   Size (in bytes)
 *                    buf:  Data
 *    Return Value:   (adr+sz) - OK, Failed Address
 */

unsigned long Verify (unsigned long adr, unsigned long sz, unsigned char *buf) {

  int i;

  SPI_FLASH_BufferRead(aux_buf, adr-base_adr, sz);
  for (i = 0; i< sz; i++) {
    if (aux_buf[i] != buf[i]) 
      return (adr+i);                   // Verification Failed (return address)
  }

  return (adr+sz);                      // Done successfully
}
