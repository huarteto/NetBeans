/***********************************************************************/
/*  This file is part of the uVision/ARM development tools             */
/*  Copyright (c) 2006-2010 Keil - An ARM Company.                     */
/*  All rights reserved.                                               */
/***********************************************************************/
/*                                                                     */
/*  SPI_Flash.c: Low level Routines for SPI Flash                      */
/*               W25Q16 programming with Nuvoton NUC501 Devices        */
/*                                                                     */
/***********************************************************************/


typedef volatile unsigned long    vu32;
typedef          unsigned long     u32;
typedef volatile unsigned char    vu8;
typedef          unsigned char     u8;

// Peripheral Memory Map
#define GCR_BASE          (0xB1000000)
#define CLK_BASE          (0xB1000200)
#define SPIM_BASE         (0xB1007000)

#define GCR               ((GCR_TypeDef  *) GCR_BASE)
#define CLK               ((CLK_TypeDef  *) CLK_BASE)
#define SPIM              ((SPIM_TypeDef *) SPIM_BASE)


// System Manager Control Registers
typedef struct {
  vu32 PDID;                 // Offset: 0x000 (R/ ) Product Identification Register
  vu32 SPOCR;                // Offset: 0x004 (R/W) System Power-On Configuration Register
  vu32 CPUCR;                // Offset: 0x008 (R/W) CPU Control Register
  vu32 MISCR;                // Offset: 0x00C (R/W) Miscellaneous Control Register
  vu32 RESERVED0[1];
  vu32 IPRST;                // Offset: 0x014 (R/W) IP Reset Control Resister
  vu32 RESERVED1[2];
  vu32 AHB_CTRL;             // Offset: 0x020 (R/W) AHB Bus Control register
  vu32 RESERVED2[3];
  vu32 PAD_REG[3];           // Offset: 0x030 (R/W) PAD function
  vu32 RESERVED3[14];
  vu32 GP_DS;                // Offset: 0x074 (R/W) GPIO pads driving strength control
} GCR_TypeDef;

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

// SPIM Control Registers
typedef struct
{
  vu32 CNTRL;                // Offset: 0x000 (R/W) Control and Status Register
  vu32 DIVIDER;              // Offset: 0x004 (R/W) Clock Divider Register
  vu32 SSR;                  // Offset: 0x008 (R/W) Slave Select Register
  vu32 RESERVED0[1];
  vu32 RX[4];                // Offset: 0x010 (R/ ) Data Receive Register
  vu32 TX[4];                // Offset: 0x020 ( /W) Data Transmit Register
  vu32 AHB_ADDR;             // Offset: 0x030 (R/W) AHB memory address
  vu32 CODE_LEN;             // Offset: 0x034 (R/W) Boot code length
  vu32 RESERVED1[2];
  vu32 SPIM_ADDR;            // Offset: 0x040 (R/W) SPI Flash Start Address
} SPIM_TypeDef;


#define CS_LOW        0
#define CS_HIGH       1


// SPI Flash Commands
#define CMD_RD        0x0B                          // High-Speed Read
#define CMD_SE        0x20                          //  4-KByte Sector Erase
#define CMD_BE        0xC7                          // Chip Erase
#define CMD_WR        0x02                          // Program One Data Byte
#define CMD_RDSR      0x05                          // Read Status Register
#define CMD_EWSR      0x50                          // Enable Write Status Register
#define CMD_WRSR      0x01                          // Write Status Register
#define CMD_WREN      0x06                          // Write Enable
#define CMD_WRDI      0x04                          // Write Disable
#define CMD_RDID      0x9F                          // JEDEC ID read


// SPI Flash Status register
#define SR_BUSY       0x01                          // Internal Write operation is in progress
#define SR_WEL        0x02                          // Device is memory Write enabled


/*----------------------------------------------------------------------------
  Initialize SPIM
 *----------------------------------------------------------------------------*/
void SPI_FLASH_DMMEnable(void) {

  SPIM->SSR      = 0x00000009;
  SPIM->CNTRL    = 0x0B321344;
  SPIM->DIVIDER  = 0x007F0001;
  SPIM->CODE_LEN = 0x00000140;
}


/*----------------------------------------------------------------------------
  Initialize SPIM
 *----------------------------------------------------------------------------*/
void SPI_FLASH_Init(void) {

  GCR->PAD_REG[1] |=  (1UL << 6);                   // SPIM0 pin enable
  CLK->AHBCLK     |=  (1UL << 7);                   // SPI FLASH/ROM Controller Clock Enable

  GCR->IPRST      |=  (1UL <<10);                   // SPIM0 and SPI1 IP Reset
  GCR->IPRST      &= ~(1UL <<10);                   // SPIM0 and SPI1 Normal operation

  SPIM->DIVIDER    =  (15  <<16) |
                      (1UL << 0) ;                  // 108MHz / (1+1) = 54MHz
  SPIM->SSR        =  (1UL << 2) | (1UL << 0);      // CS is inactive (High)

}


/*----------------------------------------------------------------------------
  SPI Flash Set CS
 *----------------------------------------------------------------------------*/
void SPI_FLASH_CS (u32 cs) {

  if (cs == CS_HIGH) {
    SPIM->SSR    =  (1UL << 2) | (1UL << 0);        // CS is inactive (High)
  }
  else {
    SPIM->SSR    =  (0UL << 2) | (1UL << 0);        // CS is   active (Low)
  }
}


/*----------------------------------------------------------------------------
  SPI Flash send a byte
 *----------------------------------------------------------------------------*/
u8 SPI_FLASH_SendByte(u8 byte) {

  while (SPIM->CNTRL & (1UL << 0));	                // wait until transfer is finished

  SPIM->TX[0]  = byte;                              // write data byte
  SPIM->CNTRL  = (8UL       <<  3)|                 // 8 Bit Transmit length
                 (1UL       <<  2)|                 // Transmit On Negative Edge  ?? -> in documentation this bit is RO
                 (1UL       <<  1)|                 // Receive On Negative Edge   ?? -> in documentation this bit is RO
                 (1UL       <<  0);                 // start transfer
  while (SPIM->CNTRL & (1UL << 0));	                // wait until transfer is finished

  return ((u8)SPIM->RX[0]);
}


/*----------------------------------------------------------------------------
  SPI Flash Enable write operation
 *----------------------------------------------------------------------------*/
void SPI_FLASH_WriteEnable(void)
{
  SPI_FLASH_CS(CS_LOW);                             // CS is   active (Low)
  
  SPI_FLASH_SendByte(CMD_WREN);                     // Send write enable
  
  SPI_FLASH_CS(CS_HIGH);                            // CS is inactive (High)
}


/*----------------------------------------------------------------------------
  SPI Flash Wait for end of write operation
 *----------------------------------------------------------------------------*/
void SPI_FLASH_WaitForWriteEnd(void) {
  u8 FLASH_Status = 0;
  
  SPI_FLASH_CS(CS_LOW);                             // CS is   active (Low)
  
  SPI_FLASH_SendByte(CMD_RDSR);                     // read status register
  
  do {                                              // do as long the memory is busy
    FLASH_Status = SPI_FLASH_SendByte(0xFF);        // read status register
  } while (FLASH_Status & SR_BUSY);                 // Write in progress

  SPI_FLASH_CS(CS_HIGH);                            // CS is inactive (High)
}


/*----------------------------------------------------------------------------
  SPI Flash Sector Erase
 *----------------------------------------------------------------------------*/
void SPI_FLASH_EraseSector(u32 addr) {

  SPI_FLASH_WriteEnable();                          // enable write operation

  // Sector Erase
  SPI_FLASH_CS(CS_LOW);                             // CS is   active (Low)

  SPI_FLASH_SendByte(CMD_SE);                       // Send Sector Erase instruction
  SPI_FLASH_SendByte((addr >> 16) & 0xFF);          // Send addr high nibble address byte
  SPI_FLASH_SendByte((addr >>  8) & 0xFF);          // Send addr medium nibble address byte
  SPI_FLASH_SendByte((addr      ) & 0xFF);          // Send addr low nibble address byte

  SPI_FLASH_CS(CS_HIGH);                            // CS is inactive (High)

  SPI_FLASH_WaitForWriteEnd();                      // wait for write end
}


/*----------------------------------------------------------------------------
  SPI Flash Sector Chip
 *----------------------------------------------------------------------------*/
void SPI_FLASH_EraseChip(void) {

  SPI_FLASH_WriteEnable();                          // enable write operation

  // Bulk Erase
  SPI_FLASH_CS(CS_LOW);                             // CS is   active (Low)

  SPI_FLASH_SendByte(CMD_BE);                       // Send Bulk Erase instruction

  SPI_FLASH_CS(CS_HIGH);                            // CS is inactive (High)

  SPI_FLASH_WaitForWriteEnd();                      // wait for write end
}


/*----------------------------------------------------------------------------
  SPI Flash Page Write
 *----------------------------------------------------------------------------*/
void SPI_FLASH_PageWrite(u8* pBuf, u32 addr, u32 sz) {

  SPI_FLASH_WriteEnable();                          // enable write operation

  SPI_FLASH_CS(CS_LOW);                             // CS is   active (Low)

  SPIM->TX[0]     = CMD_WR;		                    // Write byte instruction
  SPIM->TX[1]     = (addr >> 16) & 0xFF;		    // addr high nibble address byte
  SPIM->TX[2]     = (addr >>  8) & 0xFF;			// addr medium nibble address byte
  SPIM->TX[3]     = (addr      ) & 0xFF;            // Send addr low nibble address byte
  SPIM->SPIM_ADDR = (u32)addr;                      // SPI flash ADDR
  SPIM->AHB_ADDR  = (u32)pBuf;                      // Src Addr	
  SPIM->CODE_LEN  = sz;                             // Length
  SPIM->CNTRL     = 0x00161345;                     // start transfer
  while (SPIM->CNTRL & (1UL << 0));	                // wait until transfer is finished

  SPI_FLASH_CS(CS_HIGH);                            // CS is inactive (High)

  SPI_FLASH_WaitForWriteEnd();                      // wait for write end
}


/*----------------------------------------------------------------------------
  SPI Flash Buffer read
 *----------------------------------------------------------------------------*/
void SPI_FLASH_BufferRead (u8* pBuf, u32 addr, u32 sz) {

  SPI_FLASH_CS(CS_LOW);                             // CS is   active (Low)

  SPIM->TX[0]     = CMD_RD;		                    // Write byte instruction
  SPIM->TX[1]     = (addr >> 16) & 0xFF;		    // addr high nibble address byte
  SPIM->TX[2]     = (addr >>  8) & 0xFF;			// addr medium nibble address byte
  SPIM->TX[3]     = (addr      ) & 0xFF;            // Send addr low nibble address byte
  SPIM->SPIM_ADDR = (u32)addr;                      // SPI flash ADDR
  SPIM->AHB_ADDR  = (u32)pBuf;                      // Src Addr	
  SPIM->CODE_LEN  = sz;                             // Length
  SPIM->CNTRL     = 0x0b1a1b45;                     // start transfer   (fast Read)
  while (SPIM->CNTRL & (1UL << 0));	                // wait until transfer is finished
  
  SPI_FLASH_CS(CS_HIGH);                            // CS is inactive (High)
}


/*----------------------------------------------------------------------------
  SPI Flash Reaqd ID
 *----------------------------------------------------------------------------*/
void SPI_FLASH_ReadID(u8* pBuf, u32 sz) {

  if (sz < 3) return;

  SPI_FLASH_CS(CS_LOW);                             // CS is   active (Low)

  SPI_FLASH_SendByte(CMD_RDID);                     // Send Read ID instruction
  *(pBuf + 0) = SPI_FLASH_SendByte(0xFF);           // Read a byte from the FLASH
  *(pBuf + 1) = SPI_FLASH_SendByte(0xFF);           // Read a byte from the FLASH
  *(pBuf + 2) = SPI_FLASH_SendByte(0xFF);           // Read a byte from the FLASH

  SPI_FLASH_CS(CS_HIGH);                            // CS is inactive (High)
}
