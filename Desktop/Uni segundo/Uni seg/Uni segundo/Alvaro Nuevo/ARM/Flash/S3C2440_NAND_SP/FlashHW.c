/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashHW.C:   Hardware Layer of NAND Flash Programming Functions    */
/*               adapted for the Samsung S3C2440 Controller            */
/***********************************************************************/

#include "NAND_Error.h"                     // Error definitions

// Register and bit definitions
#define NFCONF                 (*(volatile unsigned long *) (0x4E000000))
#define NFCONT                 (*(volatile unsigned long *) (0x4E000004))
#define NFCMMD                 (*(volatile unsigned long *) (0x4E000008))
#define NFADDR                 (*(volatile unsigned long *) (0x4E00000C))
#define NFDATA                 (*(volatile unsigned long *) (0x4E000010))
#define NFDATA_C               (*(volatile unsigned char *) (0x4E000010))
#define NFMECCD0               (*(volatile unsigned long *) (0x4E000014))
#define NFMECCD1               (*(volatile unsigned long *) (0x4E000018))
#define NFSECCD                (*(volatile unsigned long *) (0x4E00001C))
#define NFSTAT                 (*(volatile unsigned long *) (0x4E000020))
#define NFESTAT0               (*(volatile unsigned long *) (0x4E000024))
#define NFESTAT1               (*(volatile unsigned long *) (0x4E000028))
#define NFMECC0                (*(volatile unsigned long *) (0x4E00002C))
#define NFMECC1                (*(volatile unsigned long *) (0x4E000030))
#define NFSECC                 (*(volatile unsigned long *) (0x4E000034))
#define NFSBLK                 (*(volatile unsigned long *) (0x4E000038))
#define NFEBLK                 (*(volatile unsigned long *) (0x4E00003C))

#define CLKCON                 (*(volatile unsigned long *) (0x4C00000C))
#define CLKSLOW                (*(volatile unsigned long *) (0x4C000010))
#define CLKDIVN                (*(volatile unsigned long *) (0x4C000014))

#define INTMSK                 (*(volatile unsigned long *) (0x4A000008))

#define GPACON                 (*(volatile unsigned long *) (0x56000000))
#define GPADAT                 (*(volatile unsigned long *) (0x56000004))

#define GPBCON                 (*(volatile unsigned long *) (0x56000010))
#define GPBDAT                 (*(volatile unsigned long *) (0x56000014))
#define GPBUP                  (*(volatile unsigned long *) (0x56000018))

// NAND Flash Commands
#define NAND_CMD_READ0         0x00         // Read mode (1) command
#define NAND_CMD_READ1         0x01         // Read mode (2) command
#define NAND_CMD_PAGEPROG      0x10         // Auto program command
#define NAND_CMD_READSTART     0x30         // Read start command
#define NAND_CMD_READ2         0x50         // Read mode (3) command
#define NAND_CMD_ERASE1ST      0x60         // Auto block erase 1-st command
#define NAND_CMD_STATUS        0x70         // Status read (1) command
#define NAND_CMD_STATUS_MULTI  0x71         // Status read (2) command
#define NAND_CMD_SDIN          0x80         // Serial data input command
#define NAND_CMD_READID        0x90         // ID read (1) command
#define NAND_CMD_ERASE2ND      0xD0         // Auto block erase 2-nd command
#define NAND_CMD_RESET         0xFF         // Reset command

// Enumerated type names definitions
enum PAGE_TYPE {SMALL_PAGE = 0, LARGE_PAGE};
enum BOOL_TYPE {     FALSE = 0,       TRUE};

// Global variables that describe interface
static volatile unsigned char bus_width_hw; // Bus width
static volatile unsigned char adr_cycles_hw;// Address word count
static volatile unsigned char page_type_hw; // Page type (small or large)

// Buffer for spare area reading or writing
unsigned char spare_buf[16];

// Module's local functions prototypes
static int SetAddress_HW      (unsigned long adr);
static int SetBlockAddress_HW (unsigned long adr);


/************************* Module Exported Functions ***************************/

/*- InitFlashController_HW (...) -----------------------------------------------
 *
 *  Initialize Flash Controller
 *    Parameter:  bus_width:  Bus Width (8, 16 bit)
 *               adr_cycles:  Addressing Cycles (3, 4, 5)
 *                page_type:  Page Type (0 - Small Page, 1 - Large Page)
 *                      clk:  Clock Frequency (Hz)
 *    Return Value:           NAND_ERROR
 */

int InitFlashController_HW (unsigned char *bus_width, unsigned char *adr_cycles, unsigned char *page_type, unsigned long clk) {

  if (NFCONF & (1 << 3)) {              // Advanced NAND Flash (Large Page)
    *bus_width   = 16;
    *page_type   = LARGE_PAGE;
    *adr_cycles  = (NFCONF & (1 << 1)) ? 5 : 4;
  } else {                              // Normal   NAND Flash (Small Page)
    *bus_width   = 8;
    *page_type   = SMALL_PAGE;
    *adr_cycles  = (NFCONF & (1 << 1)) ? 4 : 3;
  }

  // Remember parameters locally
  bus_width_hw   = *bus_width;
  adr_cycles_hw  = *adr_cycles;
  page_type_hw   = *page_type;

  // Set important registers to reset values as hardware reset is not functional 
  // through JTAG reset pin
  INTMSK  = 0xFFFFFFFF;
  CLKCON  = 0x001FFFF0;
  GPACON  = 0xFFFFFFFF;

  // Disable Write Protect, drive nFWP pin high
  GPBUP  |= 0x0080;
  GPBCON |= 0x4000;
  GPBDAT |= 0x0080;

  // Set: TACLS = 3, TWRPH0 = 7, TWRPH1 = 7
  NFCONF  = (3 << 12) | (7 << 8) | (7 << 4);

  // Set: Lock spare ECC generation, Lock main data area ECC generation, 
  //      Initialize ECC encoder/decoder, NAND flash controller enable
  NFCONT  = (1 << 6) | (1 << 5) | (1 << 4) | (1 << 0);

  while (!(NFSTAT & 0x01));             // Wait while controller busy
  NFCMMD  = NAND_CMD_RESET;             // Reset NAND flash
  while (!(NFSTAT & 0x01));             // Wait while controller busy

  NFSBLK  = (0x00000000 >> 9);          // NAND Flash prog start block address
  NFEBLK  = (0x08000000 >> 9);          // NAND Flash prog end   block address

  return (NAND_OK);
}


/*- EraseBlock_HW (...) --------------------------------------------------------
 *
 *  Erase whole Block
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   NAND_ERROR
 */

int EraseBlock_HW (unsigned long adr) {

  while (!(NFSTAT & 0x01));             // Wait while controller busy
  NFCMMD  = NAND_CMD_ERASE1ST;
  SetBlockAddress_HW(adr);
  NFCMMD  = NAND_CMD_ERASE2ND;
  while (!(NFSTAT & 0x01));             // Wait while controller busy

  return (NAND_OK);
}


/*- ReadPage_HW (...) ----------------------------------------------------------
 *
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Page Start Address
 *                     sz:  Page Size
 *                    buf:  Page Data
 *    Return Value:         NAND_ERROR
 */

int ReadPage_HW (unsigned long adr, unsigned long sz, unsigned char *buf) {
  int i;
  unsigned long *ptr_ul = (unsigned long *)buf;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    while (!(NFSTAT & 0x01));           // Wait while controller busy
    NFCMMD = NAND_CMD_READ0;
    SetAddress_HW(adr);
    while (!(NFSTAT & 0x01));           // Wait while controller busy
    for (i = 0; i < ((sz+3)>>2); i++) { // Read requested number of bytes
      *ptr_ul++ = NFDATA;
    }
    while (i < (528>>2)) {              // Read remaining bytes to the end of page
      NFDATA;
      i++;
    }
    while (!(NFSTAT & 0x01));           // Wait while controller busy
  }  
  else  {                               // Large Page Device
    return (NAND_NOT_IMPLEMENTED_ERROR);
  }

  return (NAND_OK);
}


/*- ProgramPage_HW (...) -------------------------------------------------------
 *
 *  Initialize Flash Programming Functions
 *    Parameter:      adr:  Page Start Address
 *                     sz:  Page Size
 *                    buf:  Page Data
 *    Return Value:         NAND_ERROR
 */

int ProgramPage_HW (unsigned long adr, unsigned long sz, unsigned char *buf) {
  int i;
  unsigned long *ptr_ul = (unsigned long *)buf;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    while (!(NFSTAT & 0x01));           // Wait while controller busy
    NFCMMD = NAND_CMD_SDIN;
    SetAddress_HW(adr);
    for (i = 0; i < ((sz+3)>>2); i++) { // Write requested number of bytes
      NFDATA = *ptr_ul++;
    }
    NFCMMD = NAND_CMD_PAGEPROG;
    while (!(NFSTAT & 0x01));           // Wait while controller busy
  }  
  else  {                               // Large Page Device
    return (NAND_NOT_IMPLEMENTED_ERROR);
  }

  return (NAND_OK);
}


/*- CheckBlock_HW (...) --------------------------------------------------------
 *
 *  Check if block at requested address is valid
 *    Parameter:      adr:  Block Start Address
 *    Return Value:         NAND_ERROR
 */

int CheckBlock_HW (unsigned long adr) {
  int i;
  unsigned long *ptr_ul = (unsigned long *)spare_buf;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    while (!(NFSTAT & 0x01));           // Wait while controller busy
    NFCMMD = NAND_CMD_READ2;
    SetAddress_HW(adr);
    while (!(NFSTAT & 0x01));           // Wait while controller busy
    for (i = 0; i < (16 >> 2); i++) {   // Read 16 spare area bytes
      *ptr_ul++ = NFDATA;
    }
    while (!(NFSTAT & 0x01));           // Wait while controller busy
    NFCMMD = NAND_CMD_READ0;
    while (!(NFSTAT & 0x01));           // Wait while controller busy

    // If 6-th byte in spare area is != 0xFF then the block is bad
    if (spare_buf[5] != 0xFF)          return (NAND_BAD_BLOCK_ERROR);
  }  
  else  {                               // Large Page Device
    return (NAND_NOT_IMPLEMENTED_ERROR);
  }
    
  return (NAND_OK);
}


/*- MarkBlockBad_HW (...) ------------------------------------------------------
 *
 *  Mark the block as being bad
 *    Parameter:      adr:  Block Start Address
 *    Return Value:         NAND_ERROR
 */

int MarkBlockBad_HW (unsigned long adr) {
  int i;
  unsigned long *ptr_ul = (unsigned long *)spare_buf;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    while (!(NFSTAT & 0x01));           // Wait while controller busy
    // Marking Block 0 bad is forbidden
    if (adr == 0) return (NAND_NOT_ALLOWED_ERROR);

    // Prepare buffer for spare area data
    for (i=0; i<16; i++) spare_buf[i]=0xFF;
    spare_buf[5] = 0x00;                // Bad block marker in spare area

    // Erase bad block
    EraseBlock_HW(adr);

    // Write Spare Data
    NFCMMD = NAND_CMD_READ2;
    NFCMMD = NAND_CMD_SDIN;
    SetAddress_HW(adr);
    for (i = 0; i < (16>>2); i++) {     // Write 16 spare area bytes
      NFDATA = *ptr_ul++;
    }
    NFCMMD = NAND_CMD_PAGEPROG;

    while (!(NFSTAT & 0x01));           // Wait while controller busy
    NFCMMD = NAND_CMD_READ0;
    while (!(NFSTAT & 0x01));           // Wait while controller busy
  }  
  else  {                               // Large Page Device
    return (NAND_NOT_IMPLEMENTED_ERROR);
  }

  return (NAND_OK);
}


/**************************** Auxiliary Functions ******************************/

/*- SwapU32 (...) --------------------------------------------------------------
 *
 *  Swap big <-> little-endian for 32 bit value
 *    Parameter:
 *    Return Value:         swapped value
 *//*

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
} */


/*- SetAddress_HW (...) --------------------------------------------------------
 *
 *  Set the address according to address word count
 *    Parameter:      adr:  Address Requested for Setting
 *               adr_type:  Address Word Count
 *              page_type:  Size of Page
 *    Return Value:         NAND_ERROR
 */

static int SetAddress_HW (unsigned long adr) {

  NFADDR    = adr & 0xFF;               // Address word 1st
  SetBlockAddress_HW(adr);

  return (NAND_OK);
}


/*- SetBlockAddress_HW (...) --------------------------------------------------------
 *
 *  Set the block address according to address word count
 *    Parameter:      adr:  Address Requested for Setting
 *               adr_type:  Address Word Count
 *              page_type:  Size of Page
 *    Return Value:         NAND_ERROR
 */

static int SetBlockAddress_HW (unsigned long adr) {

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    NFADDR    = (adr >>  9) & 0xFF;     // Address word 1st
    NFADDR    = (adr >> 17) & 0xFF;     // Address word 2rd
    if (adr_cycles_hw > 3)
      NFADDR  = (adr >> 25) & 0xFF;     // Address word 3th
  } 
  else  {                               // Large Page Device
    NFADDR    = (adr >> 12) & 0xFF;     // Address word 1st
    if (adr_cycles_hw > 3)
      NFADDR  = (adr >> 20) & 0xFF;     // Address word 2th
    if (adr_cycles_hw > 4)
      NFADDR  = (adr >> 28) & 0xFF;     // Address word 3th
  }

  return (NAND_OK);
}


/*******************************************************************************/
