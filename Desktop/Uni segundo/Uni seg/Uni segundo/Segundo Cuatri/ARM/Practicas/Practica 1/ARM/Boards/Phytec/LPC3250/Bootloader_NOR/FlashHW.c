/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2007                         */
/***********************************************************************/
/*                                                                     */
/*  FlashHW.C:   Hardware Layer of Flash Programming Functions         */
/*               adapted for the Philips LPC318x MLC NAND Controller   */
/***********************************************************************/

#include "LPC318x.H"                        // LPC318x definitions
#include "NAND_Error.h"                     // Error definitions

// NAND Controller Buffer Definition
#define MLC_DATA               (*(volatile unsigned long  *)(0x200B0000))
#define MLC_DATAX(x)           (*(volatile unsigned long  *)(0x200A8000+(x<<2)))

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
unsigned char spare_buf[64];

// Module's local functions prototypes
static int SetAddress_HW      (unsigned long adr);
static int SetBlockAddress_HW (unsigned long adr);


/************************* Module Exported Functions ***************************/

/*- InitFlashController_HW (...) -----------------------------------------------
 *
 *  Initialize Flash Controller
 *    Parameter:  bus_width:  Bus Width (8, 16 bit)
 *               adr_cycles:  Addressing Cycles (3, 4, 5)
 *                page_type:  Page Type (0 -Small Page, 1 - Large Page)
 *                      clk:  Clock Frequency (Hz)
 *    Return Value:           NAND_ERROR
 */

int InitFlashController_HW (unsigned char bus_width, unsigned char adr_cycles, unsigned char page_type, unsigned long clk) {
  static unsigned char first_run_f = TRUE;

  bus_width_hw    = bus_width;
  adr_cycles_hw   = adr_cycles;
  page_type_hw    = page_type;

  if (first_run_f == TRUE) {
    first_run_f = FALSE;

    FLASHCLK_CTRL = 0x00000022;         // Setup NAND Flash Clock Control
    MLC_CEH       = 0;                  // Force nCE assert
    MLC_CMD       = NAND_CMD_RESET;     // Reset NAND Flash

    while (!(MLC_ISR & 0x01));
  }

  // 16 bit bus width is not supported by controller
  if (bus_width_hw != 8) return (NAND_NOT_ALLOWED_ERROR);

  MLC_LOCK_PR     = 0xA25E;             // Unlock access to MLC_ICR register
  MLC_ICR         =  (bus_width_hw  == 16)
                  | ((adr_cycles_hw ==  4) << 1)
                  | ((page_type_hw  ==  1) << 2);

  MLC_LOCK_PR     = 0xA25E;             // Unlock access to MLC_TIME register
  MLC_TIME_REG    = (3 << 24) | (11 << 19) | (4 << 16) | (2 << 12) | (4 << 8) | (3 << 4) | (4 << 0);

  while (!(MLC_ISR & 0x01));

  return (NAND_OK);
}


/*- EraseBlock_HW (...) --------------------------------------------------------
 *
 *  Erase whole Block
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   NAND_ERROR
 */

int EraseBlock_HW (unsigned long adr) {

  while (!(MLC_ISR & 0x01));            // Wait for NAND ready
  MLC_CMD  = NAND_CMD_ERASE1ST;
  SetBlockAddress_HW(adr);
  MLC_CMD  = NAND_CMD_ERASE2ND;

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
    while (!(MLC_ISR & 0x01));          // Wait for NAND ready
    MLC_CMD = NAND_CMD_READ0;
    SetAddress_HW(adr);
    MLC_ECC_AUTO_DEC_REG = 0x00;        // Auto Decode
    while (!(MLC_ISR & 0x02));          // Wait for controller ready
    while (!(MLC_ISR & 0x04));          // Wait for ECC ready
    if (MLC_ISR & 0x40)
      return (NAND_ECC_ERROR);          // Decode ECC error
    for (i=0; i<((sz+3)>>2); i++) {     // Read requested number of bytes
      *ptr_ul++ = MLC_DATAX(i);
    }
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
    while (!(MLC_ISR & 0x01));          // Wait for NAND ready
    MLC_CMD = NAND_CMD_SDIN;
    SetAddress_HW(adr);
    MLC_ECC_ENC_REG = 0;
    for (i=0;        i<(512>>2); i++)   // Load main area data
      MLC_DATAX(i) = *ptr_ul++;
    for (i=(512>>2); i<(520>>2); i++)   // Load spare area data
      MLC_DATAX(i) = 0xFFFFFFFF;
    MLC_ECC_AUTO_ENC_REG = 0;
    while (!(MLC_ISR & 0x02));          // Wait for controller ready
    MLC_CMD     = NAND_CMD_PAGEPROG;
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
    while (!(MLC_ISR & 0x01));          // Wait for NAND ready
    MLC_CMD = NAND_CMD_READ0;
    SetAddress_HW(adr);
    MLC_ECC_AUTO_DEC_REG = 0x00;        // Auto Decode
    while (!(MLC_ISR & 0x02));          // Wait for controller ready
    while (!(MLC_ISR & 0x04));          // Wait for ECC ready
    if (MLC_ISR & 0x40)                return (NAND_ECC_ERROR);
    for (i=0;        i<(512>>2); i++)   // Load main area data
      MLC_DATAX(i);
    for (i=(512>>2); i<(520>>2); i++)   // Load spare area data
      *ptr_ul++ = MLC_DATAX(i);

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
    while (!(MLC_ISR & 0x01));          // Wait for NAND ready
    // Marking Block 0 bad is forbidden
    if (adr == 0) return (NAND_NOT_ALLOWED_ERROR);

    // Prepare buffer for spare area data
    for (i=0; i<16; i++) spare_buf[i]=0xFF;
    spare_buf[5] = 0x00;                // Bad block marker in spare area

    // Erase bad block
    EraseBlock_HW(adr);

    // Write Spare Data
    MLC_CMD = NAND_CMD_SDIN;
    SetAddress_HW(adr);
    MLC_ECC_ENC_REG = 0;
    for (i=0;        i<(512>>2); i++)   // Load main area data
      MLC_DATAX(i) = 0xFFFFFFFF;
    for (i=(512>>2); i<(520>>2); i++)   // Load spare area data
      MLC_DATAX(i) = *ptr_ul++;
    MLC_ECC_AUTO_ENC_REG = 0;
    while (!(MLC_ISR & 0x02));          // Wait for controller ready
    MLC_CMD     = NAND_CMD_PAGEPROG;
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
  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    MLC_ADDR    = (0        ) & 0xFF;   // Address word 1st
    MLC_ADDR    = (adr >>  9) & 0xFF;   // Address word 2nd
    MLC_ADDR    = (adr >> 17) & 0xFF;   // Address word 3rd
    if (adr_cycles_hw > 3)
      MLC_ADDR  = (adr >> 25) & 0xFF;   // Address word 4th
  }  
  else  {                               // Large Page Device
    MLC_ADDR    = (0        ) & 0xFF;   // Address word 1st
    MLC_ADDR    = (adr >>  8) & 0x0F;   // Address word 2nd
    MLC_ADDR    = (adr >> 12) & 0xFF;   // Address word 3rd
    if (adr_cycles_hw > 3)
      MLC_ADDR  = (adr >> 20) & 0xFF;   // Address word 4th
    if (adr_cycles_hw > 4)
      MLC_ADDR  = (adr >> 28) & 0xFF;   // Address word 5th
  }
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
    MLC_ADDR    = (adr >>  9) & 0xFF;   // Address word 1nd
    MLC_ADDR    = (adr >> 17) & 0xFF;   // Address word 2rd
    if (adr_cycles_hw > 3)
      MLC_ADDR  = (adr >> 25) & 0xFF;   // Address word 3th
  }  
  else  {                               // Large Page Device
    MLC_ADDR    = (adr >> 12) & 0xFF;   // Address word 1rd
    if (adr_cycles_hw > 3)
      MLC_ADDR  = (adr >> 20) & 0xFF;   // Address word 2th
    if (adr_cycles_hw > 4)
      MLC_ADDR  = (adr >> 28) & 0xFF;   // Address word 3th
  }
  return (NAND_OK);
}


/*******************************************************************************/
