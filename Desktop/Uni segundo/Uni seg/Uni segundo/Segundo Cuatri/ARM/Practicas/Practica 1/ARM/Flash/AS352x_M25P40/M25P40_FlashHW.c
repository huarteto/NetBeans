/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2005 - 2007                                */
/******************************************************************************/
/*                                                                            */
/*  M25P40_FlashHW.C:   Hardware Layer of External SPI Flash ST M25P40        */
/*                      Programming Functions with AustriaMicroSystems        */
/*                      AS352x Devices                                        */
/*                                                                            */
/******************************************************************************/

#include <AS352x.H>                     // AS352x definitions
#include "Error.h"                      // Error definitions

#define SYS_CLOCK       24000000        // System Clock (Hz)
#define SPI_CLOCK       12000000        // Expected SPI clock speed (Hz)
#define SPI_SPEED       6000000         // Expected SPI communication speed (Hz)

// Constants depending on Chip used
#define PAGE_SIZE       256             // Size of page
#define CHIP_SIZE       0x80000         // Chip size (in bytes) = 512kB

#define BLOCK_SIZE      256*PAGE_SIZE   // Block size (used for erase)

// Flash Commands
#define CMD_WREN        0x06            // Write enable
#define CMD_WRDI        0x04            // Write disable
#define CMD_RDID        0x9F            // Read identification
#define CMD_RDSR        0x05            // Read status register
#define CMD_WRSR        0x01            // Write status register
#define CMD_READ        0x03            // Read data bytes
#define CMD_FAST_READ   0x0B            // Read data bytes at higher speed
#define CMD_PP          0x02            // Page program
#define CMD_SE          0xD8            // Sector erase
#define CMD_BE          0xC7            // Bulk erase
#define CMD_DP          0xB9            // Deep power-down
#define CMD_RES         0xAB            // Release from deep power-down


static unsigned char    command[8];     // Command

// Module's local functions prototypes
static unsigned long    SwapU32         (unsigned long to_swap);
static          void    WriteEnable     (void);
static          int     SendCommand     (unsigned char cmd, unsigned char cmd_sz, unsigned long adr, unsigned char *buf, unsigned long sz);


/************************* Module Exported Functions ***************************/

/*- InitFlashController_HW (...) -----------------------------------------------
 *
 *  Initialize Flash Controller
 *    Parameter:  bus_width:  Bus Width (8, 16 bit)
 *               adr_cycles:  Addressing Cycles (3, 4, 5)
 *                page_type:  Page Type (0 -Small Page, 1 - Large Page)
 *                      clk:  Clock Frequency (Hz)
 *    Return Value:           ERROR code
 */

int InitFlashController_HW (unsigned char bus_width, unsigned char adr_cycles, unsigned char page_type, unsigned long clk) {

  unsigned int i;

  // 1. Initialize pins for SPI functionality

  // Enable clocks (GPIO3, SSP)
  Reg32(CGU_REG_PERI) |= (1 << 10) | (1 << 16);

  // Setup pins
  Reg32(GPIO3_AFSEL)  &= ~(1 << 4);     // xpc[4] clear alternate function
  Reg32(GPIO3_DIR)    |=  (1 << 4);     // xpc[4] set as output
  Reg32(GPIO3_DATA + (0x10<<2)) = 1<<4; // Deselect flash

  // 2. Initialize SPI controller
  Reg32(CCU_IO)       &= ~(1 << 4);
  Reg32(SSP_SSPCPSR)   = SYS_CLOCK / SPI_CLOCK;
  Reg32(SSP_SSPCR0)    = (((SPI_CLOCK / SPI_SPEED) - 1) << 8) | 0x07;
  Reg32(SSP_SSPCR1)   &= ~(1 << 1);     // SPI operation disable
  Reg32(SSP_SSPCR1)   &= ~(1 << 2);     // Select master
  Reg32(SSP_SSPCR0)   &= ~(0x03 << 6);  // Clear SPO, SPH

  Reg32(GPIO3_DATA + (0x10<<2)) = 0;    // Select flash
  for (i =0; i < 20; i++);              // Wait to select flash
  Reg32(GPIO3_DATA + (0x10<<2)) = 1<<4; // Deselect flash

  Reg32(SSP_SSPCR1)   |=  (1 << 1);     // SPI operation enable

  // Wait for Flash to become ready
  do { SendCommand(CMD_RDSR, 2, 0, 0, 0); }  while (command[1] & 0x01);

  return (OK);
}


/*- GetPageSize_HW (...) -------------------------------------------------------
 *
 *  Get Size of Page
 *    Parameter:
 *    Return Value:           page size in bytes
 */

int GetPageSize_HW (void) {

  return (PAGE_SIZE);
}


/*- ErasePage_HW (...) ---------------------------------------------------------
 *
 *  Erase a Page
 *    Parameter:        adr:  Page Start Address
 *    Return Value:           ERROR code
 */

int ErasePage_HW (unsigned long adr) {

  return (ERROR_NOT_IMPLEMENTED);
}


/*- EraseBlock_HW (...) ---------------------------------------------------------
 *
 *  Erase a Block
 *    Parameter:        adr:  Block Start Address
 *    Return Value:           ERROR code
 */

int EraseBlock_HW (unsigned long adr) {

  WriteEnable();                        // Enable write to flash

  if (SendCommand(CMD_SE, 4, adr, 0, 0) != OK)
    return (ERROR_ERASE);

  // Wait for operation to finish
  do { SendCommand(CMD_RDSR, 2, 0, 0, 0); }  while (command[1] & 0x01);

  return (OK);
}


/*- EraseChip_HW (...) ---------------------------------------------------------
 *
 *  Erase Full Flash Memory
 *    Parameter:
 *    Return Value:           ERROR code
 */

int EraseChip_HW (void) {

  WriteEnable();                        // Enable write to flash

  if (SendCommand(CMD_BE, 1, 0, 0, 0) != OK)
    return (ERROR_ERASE);

  // Wait for operation to finish
  do { SendCommand(CMD_RDSR, 2, 0, 0, 0); }  while (!(command[1] & 0x01));

  return (OK);
}


/*- ReadPage_HW (...) ----------------------------------------------------------
 *
 *  Initialize Flash Programming Functions
 *    Parameter:        adr:  Page Start Address
 *                       sz:  Page Size
 *                      buf:  Page Data
 *    Return Value:           ERROR code
 */

int ReadPage_HW (unsigned long adr, unsigned long sz, unsigned char *buf) {

  if (SendCommand(CMD_READ, 4, adr, buf, sz) != OK)
    return (ERROR_READ);

  return (OK);
}


/*- ProgramPage_HW (...) -------------------------------------------------------
 *
 *  Initialize Flash Programming Functions
 *    Parameter:        adr:  Page Start Address
 *                       sz:  Page Size
 *                      buf:  Page Data
 *    Return Value:           ERROR code
 */

int ProgramPage_HW (unsigned long adr, unsigned long sz, unsigned char *buf) {

  unsigned int i;

  WriteEnable();                        // Enable write to flash

  // Clear (0xFF) rest of a page if not full page is requested for program 
  // because always a whole page must be programmed
  if (sz < PAGE_SIZE) 
    for (i = sz; i < PAGE_SIZE; i++)
      buf[i] = 0xFF;

  // Write Page
  if (SendCommand(CMD_PP, 4, adr, buf, PAGE_SIZE) != OK)
    return (ERROR_WRITE);

  // Wait for operation to finish
  do { SendCommand(CMD_RDSR, 2, 0, 0, 0); }  while (command[1] & 0x01);

  return (OK);
}


/*- CheckBlock_HW (...) --------------------------------------------------------
 *
 *  Check if block at requested address is valid
 *    Parameter:        adr:  Block Start Address
 *    Return Value:           ERROR code
 */

int CheckBlock_HW (unsigned long adr) {

  return (ERROR_NOT_IMPLEMENTED);
}


/*- MarkBlockBad_HW (...) ------------------------------------------------------
 *
 *  Mark the block as being bad
 *    Parameter:        adr:  Block Start Address
 *    Return Value:           ERROR code
 */

int MarkBlockBad_HW (unsigned long adr) {

  return (ERROR_NOT_IMPLEMENTED);
}


/**************************** Auxiliary Functions ******************************/

/*- SwapU32 (...) --------------------------------------------------------------
 *
 *  Swap big <-> little-endian for 32 bit value
 *    Parameter:
 *    Return Value:           swapped value
 */

static unsigned long SwapU32(unsigned long to_swap)
{
  const unsigned long mask = 0x00FF00FF;
  unsigned long temp;

  __asm {
    AND temp,    mask,    to_swap
    AND to_swap, mask,    to_swap, ROR #24
    ORR to_swap, to_swap, temp,    ROR #8
  }

  return (to_swap);
} 


/*- WriteEnable (...) ----------------------------------------------------------
 *
 *  Enable write to flash
 *    Parameter:
 *    Return Value:
 */

void WriteEnable (void) {

  SendCommand(CMD_WREN, 1, 0, 0, 0);

  // Wait for operation to finish
  do { SendCommand(CMD_RDSR, 2, 0, 0, 0); }  while (!(command[1] & 0x02));
}


/*- SendCommandDF (...) --------------------------------------------------------
 *
 *  Send command to SPI Flash
 *    Parameter:        cmd:  Command to be Sent
 *                   cmd_sz:  Command Size (in bytes)
 *                      adr:  Address in Flash
 *                      buf:  Data to be Sent (or Received)
 *                       sz:  Size of Data
 *    Return Value:           ERROR code
 */

static int SendCommand (unsigned char cmd, unsigned char cmd_sz, unsigned long adr, unsigned char *buf, unsigned long sz) {

  unsigned int i, j = 0;

  Reg32(GPIO3_DATA + (0x10<<2)) = 0;    // Select flash

  command[0] = cmd;
  command[1] = (adr >> 16);
  command[2] = (adr >> 8);
  command[3] =  adr;
  command[4] = 0;

  for (i = 0; i < cmd_sz; i++) {
    while (!(Reg32(SSP_SSPSR)&(1<<0))); // Wait for Tx FIFO to become empty
    Reg8(SSP_SSPTXD) = command[i];
    while (!(Reg32(SSP_SSPSR)&(1<<2))); // Wait for Rx FIFO receive something
    command[i] = Reg8(SSP_SSPRXD);
  }

  for (i = 0; i < sz; i++) {
    while (!(Reg32(SSP_SSPSR)&(1<<0))); // Wait for Tx FIFO to become empty
    Reg8(SSP_SSPTXD) = *buf;
    while (!(Reg32(SSP_SSPSR)&(1<<2))); // Wait for Rx FIFO receive something
    *buf = Reg8(SSP_SSPRXD);
    buf++;
  }

  Reg32(GPIO3_DATA + (0x10<<2)) = 1<<4; // Deselect flash
  for (i = 0; i < 20; i++);

  return (OK);
}


/*******************************************************************************/
