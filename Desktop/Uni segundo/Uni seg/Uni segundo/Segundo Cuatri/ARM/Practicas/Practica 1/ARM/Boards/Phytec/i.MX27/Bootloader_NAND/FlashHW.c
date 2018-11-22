/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                                */
/******************************************************************************/
/*                                                                            */
/*  FlashHW.C:   Hardware Layer of Flash Programming Functions                */
/*               adapted for the Freescale i.MX27 NAND Flash Controller       */
/*                                                                            */
/******************************************************************************/

#include <iMX27.h>                      /* i.MX27 definitions                 */
#include "NAND_Error.h"                 /* Error definitions                  */

// Register Definitions
#define _REG8(x)              (*((volatile unsigned char  *) x))
#define _REG16(x)             (*((volatile unsigned short *) x))
#define _REG32(x)             (*((volatile unsigned long  *) x))

// Bit Constants
#define NFC_RST               (1 <<  6)
#define ECC_EN                (1 <<  3)
#define SP_EN                 (1 <<  2)
#define INT                   (1 << 15)
#define FDO_DATA              (1 <<  3)
#define FDI                   (1 <<  2)
#define FADD                  (1 <<  1)
#define FCMD                  (1 <<  0)

// NAND Controller Buffer Definition
// Main Area Buffers
#define MAB0                  (0xD8000000UL)
#define MAB1                  (0xD8000200UL)
#define MAB2                  (0xD8000400UL)
#define MAB3                  (0xD8000600UL)
// Spare Area Buffers
#define SAB0                  (0xD8000800UL)
#define SAB1                  (0xD8000810UL)
#define SAB2                  (0xD8000820UL)
#define SAB3                  (0xD8000830UL)

// NAND Flash Commands
#define NAND_CMD_READ0        0x00          // Read mode (1) command
#define NAND_CMD_READ1        0x01          // Read mode (2) command
#define NAND_CMD_PAGEPROG     0x10          // Auto program command
#define NAND_CMD_READSTART    0x30          // Read start command
#define NAND_CMD_READ2        0x50          // Read mode (3) command
#define NAND_CMD_ERASE1ST     0x60          // Auto block erase 1-st command
#define NAND_CMD_STATUS       0x70          // Status read (1) command
#define NAND_CMD_STATUS_MULTI 0x71          // Status read (2) command
#define NAND_CMD_SDIN         0x80          // Serial data input command
#define NAND_CMD_READID       0x90          // ID read (1) command
#define NAND_CMD_ERASE2ND     0xD0          // Auto block erase 2-nd command
#define NAND_CMD_RESET        0xFF          // Reset command

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
static void NANDCmd_HW         (unsigned char cmd);
static void NANDDat_HW         (unsigned char cmd);
static void SetAddress8bits_HW (unsigned char val);
static void SetAddress_HW      (unsigned long adr);
static void SetRowAddress_HW   (unsigned long adr);


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

int InitFlashController_HW (unsigned char bus_width, unsigned char adr_cycles, unsigned char page_type, unsigned long clk) {

  bus_width_hw     = bus_width;
  adr_cycles_hw    = adr_cycles;
  page_type_hw     = page_type;

  if (NFC_UNLOCK_END_BLK_ADD != 0xFFFF) {
    // If hardware has not been initialized, initialize it only once

    AIPI1_PSR0       = 0x00040304;
    AIPI1_PSR1       = 0xFFFBFCFB;
    AIPI1_PAR        = 0xFFDFFFFF;
    AIPI2_PSR0       = 0x3FDC0000;
    AIPI2_PSR1       = 0x3FFFFDFF;
    AIPI2_PAR        = 0xFFFFFF7F;

    SYSCTRL_FMCR            &= ~((1 << 5) | (1 << 4));
    NFC_UNLOCK_START_BLK_ADD =  0x0000;
    NFC_UNLOCK_END_BLK_ADD   =  0xFFFF;
    NFC_NF_WR_PROT           =  4;
    NFC_NFC_CONFIGURATION    =  2;
    NFC_NAND_FLASH_CONFIG1   =  ECC_EN | NFC_RST;
    NFC_NAND_FLASH_CONFIG1   =  ECC_EN ;
    NFC_NAND_FLASH_CONFIG2   =  0x8000;
  }

  // 16 bit bus width is not supported by controller
  if (bus_width_hw != 8) return (NAND_NOT_ALLOWED_ERROR);

  return (NAND_OK);
}


/*- EraseBlock_HW (...) --------------------------------------------------------
 *
 *  Erase whole Block
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   NAND_ERROR
 */

int EraseBlock_HW (unsigned long adr) {

  NANDCmd_HW (NAND_CMD_ERASE1ST);
  SetRowAddress_HW(adr);
  NANDCmd_HW (NAND_CMD_ERASE2ND);

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
  unsigned short *ptr_mab0 = (unsigned short *) MAB0;
  unsigned short *ptr_sab0 = (unsigned short *) SAB0;
  unsigned short *ptr_dest = (unsigned short *) buf;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    NANDCmd_HW (NAND_CMD_READ0);
    SetAddress_HW(adr);
    NANDDat_HW (FDO_DATA);
    if (NFC_ECC_STATUS_RESULT & 0x0A)
      return (NAND_ECC_ERROR);          // Decode ECC error
    if (NFC_ECC_STATUS_RESULT & 0x05) {
      // NFC has corrected correctable errors and written them 
      // back to NAND flash
      if (NFC_ECC_STATUS_RESULT & 0x04){// 1-bit error in Main Area, correct it
        *(ptr_mab0+(NFC_ECC_RSLT_MAIN_AREA  >> 4)) ^= 
                                        1 << (NFC_ECC_RSLT_MAIN_AREA  & 0x0F);
      }
      if (NFC_ECC_STATUS_RESULT & 0x01){// 1-bit error in Spare Area, correct it
        *(ptr_sab0+(NFC_ECC_RSLT_SPARE_AREA >> 4)) ^= 
                                        1 << (NFC_ECC_RSLT_SPARE_AREA & 0x0F);
      }
    }
    for (i=0; i<((sz+3)>>1); i++) {     // Read requested number of bytes
      *ptr_dest++ = *ptr_mab0++;
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
  unsigned short *ptr_sab0 = (unsigned short *) SAB0;
  unsigned short *ptr_src  = (unsigned short *) buf;
  unsigned short *ptr_dest = (unsigned short *) MAB0;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    for (i=(512>>1); i<(528>>1); i++)   // Clear spare area
      *ptr_sab0++ = 0xFFFF;
    for (i=0;        i<(512>>1); i++)   // Load main area data
      *ptr_dest++ = *ptr_src++;
    NANDCmd_HW (NAND_CMD_SDIN);
    SetAddress_HW(adr);
    NANDDat_HW (FDI);
    NANDCmd_HW (NAND_CMD_PAGEPROG);
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
  unsigned short *ptr_sab0 = (unsigned short *) SAB0;
  unsigned short *ptr_dest = (unsigned short *) spare_buf;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    NANDCmd_HW (NAND_CMD_READ0);
    SetAddress_HW(adr);
    NANDDat_HW (FDO_DATA);
    for (i=(512>>1); i<(528>>1); i++) { // Load spare area data
      *ptr_dest++ = *ptr_sab0++;
    }

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
  unsigned short *ptr_src  = (unsigned short *) spare_buf;
  unsigned short *ptr_dest = (unsigned short *) SAB0;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    // Marking Block 0 bad is forbidden
    if (adr == 0) return (NAND_NOT_ALLOWED_ERROR);

    // Prepare buffer for spare area data
    for (i=0; i<16; i++) spare_buf[i]=0xFF;
    spare_buf[5] = 0x00;                // Bad block marker in spare area

    // Erase bad block
    EraseBlock_HW(adr);

    for (i=(512>>1); i<(528>>1); i++) { // Load spare area data
      *ptr_dest++ = *ptr_src++;
    }
    NANDCmd_HW (NAND_CMD_SDIN);
    SetAddress_HW(adr);
    NANDDat_HW (FDI);
    NANDCmd_HW (NAND_CMD_PAGEPROG);
  }  
  else  {                               // Large Page Device
    return (NAND_NOT_IMPLEMENTED_ERROR);
  }

  return (NAND_OK);
}


/**************************** Auxiliary Functions ******************************/

/*- NANDCmd_HW (...) -----------------------------------------------------------
 *
 *  Execute NAND Flash controller command
 *    Parameter:      cmd:  Command Requested
 *    Return Value:         0 - Fail, 1 - OK
 */

static void NANDCmd_HW (unsigned char cmd) {

  NFC_NAND_FLASH_CMD     = cmd;
  NFC_NAND_FLASH_CONFIG2 = FCMD;
  while (!(NFC_NAND_FLASH_CONFIG2 & INT));
}


/*- NANDDat_HW (...) -----------------------------------------------------------
 *
 *  Execute NAND Flash controller data command
 *    Parameter:      cmd:  Data Command Requested
 *    Return Value:         0 - Fail, 1 - OK
 */

static void NANDDat_HW (unsigned char cmd) {

  NFC_NAND_FLASH_CONFIG2 = cmd;
  while (!(NFC_NAND_FLASH_CONFIG2 & INT));
}


/*- SetAddress8bits_HW (...) ---------------------------------------------------
 *
 *  Set 8 bits value as address (used for multi cycle addressing)
 *    Parameter:      val:  Address Requested for Setting
 *    Return Value:         none
 */

static void SetAddress8bits_HW (unsigned char val) {

  NFC_NAND_FLASH_ADD     = val;
  NFC_NAND_FLASH_CONFIG2 = FADD;
  while (!(NFC_NAND_FLASH_CONFIG2 & INT));
}


/*- SetAddress_HW (...) --------------------------------------------------------
 *
 *  Set the address according to address word count
 *    Parameter:      adr:  Address Requested for Setting
 *              page_type:  Size of Page (global variable)
 *    Return Value:         void
 */

static void SetAddress_HW (unsigned long adr) {
  SetAddress8bits_HW (adr);                 // Column address word 1st
  if (page_type_hw == LARGE_PAGE)           // Large Page Device
    SetAddress8bits_HW ((adr >> 8) & 0x0F); // Column address word 2nd
  SetRowAddress_HW (adr);
}


/*- SetRowAddress_HW (...) -----------------------------------------------------
 *
 *  Set the row address according to address word count
 *    Parameter:      adr:  Address Requested for Setting
 *               adr_type:  Address Word Count (global variable)
 *              page_type:  Size of Page (global variable)
 *    Return Value:         none
 */

static void SetRowAddress_HW (unsigned long adr) {
  unsigned long adr_cyc = adr_cycles_hw;

  adr >>= 9;
  if (page_type_hw == LARGE_PAGE) {         // Large Page Device
    adr >>= 3;
    adr_cyc --;
  }

  SetAddress8bits_HW (adr);                 // Row address word 1st
  adr >>= 8;
  SetAddress8bits_HW (adr);                 // Row address word 2nd
  if (adr_cyc > 3) {
    adr >>= 8;
    SetAddress8bits_HW (adr);               // Row address word 3rd
  }
}


/*******************************************************************************/
