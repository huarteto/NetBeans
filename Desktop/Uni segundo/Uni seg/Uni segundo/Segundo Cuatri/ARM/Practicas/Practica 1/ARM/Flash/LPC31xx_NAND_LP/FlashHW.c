/******************************************************************************/
/*  This file is part of the ARM Toolchain package                            */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2009                                */
/******************************************************************************/
/*                                                                            */
/*  FlashHW.C:   Hardware Layer of Flash Programming Functions adapted for    */
/*               the NXP LPC313x Device Series NAND Flash Controller          */
/*                                                                            */
/******************************************************************************/

#include "string.h"
#include "LPC313x.H"                        // LPC313x definitions
#include "NAND_Error.h"                     // Error definitions

// Frequency of CPU controller clock
#define CPU_CLK                180000000UL  // In Hz
#define FOR_IN_US(x)           (((CPU_CLK/1000000UL)*x) >> 2)

// NAND Controller Buffer Definition
#define NAND_BUF0              ((volatile unsigned char *)(0x70000000))
#define NAND_BUF1              ((volatile unsigned char *)(0x70000400))

// NAND Flash Commands
#define NAND_CMD_READ          0x00         // Read command
#define NAND_CMD_PAGEPROG      0x10         // Auto program command
#define NAND_CMD_READSTART     0x30         // Read start command
#define NAND_CMD_ERASE1ST      0x60         // Auto block erase 1-st command
#define NAND_CMD_STATUS        0x70         // Status read command
#define NAND_CMD_SDIN          0x80         // Serial data input command
#define NAND_CMD_READID        0x90         // ID read command
#define NAND_CMD_ERASE2ND      0xD0         // Auto block erase 2-nd command
#define NAND_CMD_RESET         0xFF         // Reset command

// Enumerated type names definitions
enum PAGE_TYPE {SMALL_PAGE = 0, LARGE_PAGE};

// Global variables that describe interface
static volatile unsigned char bus_width_hw; // Bus width
static volatile unsigned char adr_cycles_hw;// Address word count
static volatile unsigned char page_type_hw; // Page type (small or large)

// Buffer for spare area reading or writing
unsigned char bad_block;

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
         unsigned int  i;

  bus_width_hw    = bus_width;
  adr_cycles_hw   = adr_cycles;
  page_type_hw    = page_type;

  bad_block = 0;

  if (first_run_f == TRUE) {
    first_run_f = FALSE;

    CGU_SB->clk_pcr[CGU_SB_NANDFLASH_S0_CLK_ID]   = 7;
    CGU_SB->clk_pcr[CGU_SB_NANDFLASH_ECC_CLK_ID]  = 7;
    CGU_SB->clk_pcr[CGU_SB_NANDFLASH_NAND_CLK_ID] = 7;
    CGU_SB->clk_pcr[CGU_SB_NANDFLASH_PCLK_ID]     = 7;
    CGU_CFG->nandflash_ctrl_ecc_reset_n_soft      = 0;
    CGU_CFG->nandflash_ctrl_nand_reset_n_soft     = 0;
    for (i = 0; i < FOR_IN_US(25); i++);
    CGU_CFG->nandflash_ctrl_ecc_reset_n_soft      = 1;
    CGU_CFG->nandflash_ctrl_nand_reset_n_soft     = 1;
    SYS_REGS->mux_nand_mci_sel                    = 0;

    NAND_CTRL->set_ce  = NAND_SETCE_WP | NAND_SETCE_CV(0);
    NAND_CTRL->timing1 = NAND_TIM1_TSRD(2) |
                         NAND_TIM1_TALS(2) |
                         NAND_TIM1_TALH(0) |
                         NAND_TIM1_TCLS(2) |
                         NAND_TIM1_TCLH(0) ;
    NAND_CTRL->timing2 = NAND_TIM2_TDRD(2) |
                         NAND_TIM2_TEBI(0) |
                         NAND_TIM2_TCH(0)  |
                         NAND_TIM2_TCS(3)  |
                         NAND_TIM2_TRH(1)  |
                         NAND_TIM2_TRP(3)  |
                         NAND_TIM2_TWH(1)  |
                         NAND_TIM2_TWP(2)  ;
    NAND_CTRL->config  = NAND_CFG_EC | NAND_CFG_DC;
    NAND_CTRL->set_cmd = NAND_CMD_RESET;
    for (i = 0; i < FOR_IN_US(1000); i++);
  }

  if (bus_width_hw == 16)
    NAND_CTRL->config |=  NAND_CFG_WD;
  else if (bus_width_hw == 8)
    NAND_CTRL->config &= ~NAND_CFG_WD;

  return (NAND_OK);
}


/*- EraseBlock_HW (...) --------------------------------------------------------
 *
 *  Erase whole Block
 *    Parameter:      adr:  Block Start Address
 *    Return Value:   NAND_ERROR
 */

int EraseBlock_HW (unsigned long adr) {
  int i;

  NAND_CTRL->set_cmd = NAND_CMD_ERASE1ST;
  SetBlockAddress_HW(adr);
  NAND_CTRL->set_cmd = NAND_CMD_ERASE2ND;
  do {
    NAND_CTRL->set_cmd = NAND_CMD_STATUS;
  } while (!(NAND_CTRL->read_data & (1 << 5)));
  for (i = 0; i < FOR_IN_US(1000); i++);

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
  int i, cnt   = 0;
  int ecc_fail = 0;
  int bad_blk  = 0;

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    return (NAND_NOT_IMPLEMENTED_ERROR);
  }  
  else  {                               // Large Page Device
    if (sz > 2048) sz = 2048;
    NAND_CTRL->set_cmd = NAND_CMD_READ;
    SetAddress_HW(adr);
    NAND_CTRL->set_cmd = NAND_CMD_READSTART;
    for (i = 0; i < FOR_IN_US(2000); i++);
    do {
      cnt ++;
      NAND_CTRL->irq_status_raw = 0xFFFFFFFF;
      NAND_CTRL->control_flow   = 1;
      for (i = 0; i < FOR_IN_US(10000); i++);
      while (!(NAND_CTRL->irq_status_raw & NAND_IRQ_RD_RAM0));
      while (!(NAND_CTRL->irq_status_raw & NAND_IRQ_ECC_DEC_RAM0));
      if (*(NAND_BUF0+515) == 0x00) {
        /* If bad block as marked by our algorithm byte 515 is 0x00 */
        bad_blk = 1;
      }
      if (sz > 512) {
        if (buf != NULL) {
          memcpy((void *)buf, (void *) NAND_BUF0, 512);
          buf += 512;
        }
        sz -= 512;
      } else {
        if (buf != NULL) {
          memcpy((void *)buf, (void *) NAND_BUF0, sz);
        }
        sz  = 0;
      }
      if (NAND_CTRL->irq_status_raw & NAND_IRQ_RD_RAM0) {
        if (NAND_CTRL->irq_status_raw & NAND_IRQ_ERR_UNR_RAM0) {
          for (i = 0; i < 528; i++) {
            if (*(NAND_BUF0 + i) != 0xFF) {
              if ((cnt == 4) && (*(NAND_BUF0+464) == 0x00)) {
                /* If bad block as marked by vendor of NAND chip 
                   position 2048 (first spare byte in large page) is 0x00 */
                bad_blk = 1;
              }
              ecc_fail = 1;
            }
          }
        }
      } else {
        return (NAND_INTERFACE_ERROR);  // Not expected error
      }
    } while (sz);      
  }
  if (bad_blk)
    return (NAND_BAD_BLOCK_ERROR);
  if (ecc_fail)
    return (NAND_ECC_ERROR);

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

  if (page_type_hw == SMALL_PAGE)  {    // Small Page Device
    return (NAND_NOT_IMPLEMENTED_ERROR);
  }  
  else  {                               // Large Page Device
    if (sz > 2048) sz = 2048;
    NAND_CTRL->set_cmd = NAND_CMD_SDIN;
    SetAddress_HW(adr);
    do {
      NAND_CTRL->irq_status_raw = 0xFFFFFFFF;
      if (sz > 512) {
        if (buf != NULL) {
          memcpy((void *) NAND_BUF0, (void *)buf, 512);
        } else {
          memset((void *) NAND_BUF0, 0xFF, 512);
        }
        if (bad_block) {
          bad_block = 0;
          memset((void *)(NAND_BUF0 + 512), 0xFF, 3);
          *((unsigned char *)NAND_BUF0 + 515) = 0x00;
        } else {
          memset((void *)(NAND_BUF0 + 512), 0xFF, 4);
        }
        buf += 512;
        sz  -= 512;
      } else {
        if (buf != NULL) {
          memcpy((void *) NAND_BUF0, (void *)buf, sz);
        } else {
          memset((void *) NAND_BUF0, 0xFF, sz);
        }
        if (bad_block) {
          bad_block = 0;
          memset((void *)(NAND_BUF0 + sz), 0xFF, 515 - sz);
          *(NAND_BUF0 + 515) = 0x00;
        } else {
          memset((void *)(NAND_BUF0 + sz), 0xFF, 516 - sz);
        }
        sz  = 0;
      }
      while (!(NAND_CTRL->irq_status_raw & NAND_IRQ_ECC_ENC_RAM0));
      NAND_CTRL->control_flow = (1 << 4);
      while (!(NAND_CTRL->irq_status_raw & NAND_IRQ_WR_RAM0));
    } while (sz);      
    NAND_CTRL->set_cmd = NAND_CMD_PAGEPROG;
    for (i = 0; i < FOR_IN_US(5000); i++);
    do {
      NAND_CTRL->set_cmd = NAND_CMD_STATUS;
    } while (!(NAND_CTRL->read_data & (1 << 5)));
    if (NAND_CTRL->read_data & 1)
      return (NAND_INTERFACE_ERROR);
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

  if (ReadPage_HW (adr, 512, NULL) == NAND_BAD_BLOCK_ERROR) {
    return (NAND_BAD_BLOCK_ERROR);
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

  bad_block = 1;
  if (ProgramPage_HW (adr, 512, NULL) != NAND_OK) {
    return (NAND_INTERFACE_ERROR);
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
    NAND_CTRL->set_addr   = (0        ) & 0xFF;   // Address word 1st
    NAND_CTRL->set_addr   = (adr >>  9) & 0xFF;   // Address word 2nd
    NAND_CTRL->set_addr   = (adr >> 17) & 0xFF;   // Address word 3rd
    if (adr_cycles_hw > 3)
      NAND_CTRL->set_addr = (adr >> 25) & 0xFF;   // Address word 4th
  }  
  else  {                               // Large Page Device
    NAND_CTRL->set_addr   = (adr      ) & 0xFF;   // Address word 1st
    NAND_CTRL->set_addr   = (adr >>  8) & 0x07;   // Address word 2nd
    NAND_CTRL->set_addr   = (adr >> 11) & 0xFF;   // Address word 3rd
    if (adr_cycles_hw > 3)
      NAND_CTRL->set_addr = (adr >> 19) & 0xFF;   // Address word 4th
    if (adr_cycles_hw > 4)
      NAND_CTRL->set_addr = (adr >> 27) & 0xFF;   // Address word 5th
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
    NAND_CTRL->set_addr   = (adr >>  9) & 0xFF;   // Address word 1nd
    NAND_CTRL->set_addr   = (adr >> 17) & 0xFF;   // Address word 2rd
    if (adr_cycles_hw > 3)
      NAND_CTRL->set_addr = (adr >> 25) & 0xFF;   // Address word 3th
  }  
  else  {                               // Large Page Device
    NAND_CTRL->set_addr   = (adr >> 11) & 0xC0;   // Address word 1rd
    if (adr_cycles_hw > 3)
      NAND_CTRL->set_addr = (adr >> 19) & 0xFF;   // Address word 2th
    if (adr_cycles_hw > 4)
      NAND_CTRL->set_addr = (adr >> 27) & 0xFF;   // Address word 3th
  }

  return (NAND_OK);
}


/*******************************************************************************/
