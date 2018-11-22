/***********************************************************************
 * $Id:: lpc313x_nand_driver.c 1529 2009-01-06 02:08:40Z pdurgesh       $
 *
 * Project: LPC313x NAND controller driver
 *
 * Description:
 *     This file contains driver support for the LPC313x NAND.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 *********************************************************************/

#include "lpc313x_sysreg.h"
#include "lpc313x_nand_driver.h"
#include "lpc313x_cgu_driver.h"

/***********************************************************************
 * WDT driver package data
***********************************************************************/

/* NAND device configuration structure type */
typedef struct
{
  NAND_FLASH_CTRL_REGS_T *regptr;     /* Pointer to NAND registers */
  BOOL_32 init;           /* Device initialized flag */
} NAND_DRV_T;

/* WDT driver data */
static NAND_DRV_T g_nand_drv;

/***********************************************************************
 * NAND driver private functions
 **********************************************************************/

/***********************************************************************
 * NAND driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: nand_open
 *
 * Purpose: Open the NAND
 *
 * Processing:
 *     If the passed register base is valid and the NAND
 *     driver isn't already initialized, then setup the NAND
 *     into a default initialized state that is disabled. Return
 *     a pointer to the driver's data structure or NULL if driver
 *     initialization failed.
 *
 * Parameters:
 *     ipbase: Pointer to a NAND peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a NAND config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_open(void *ipbase, INT_32 arg)
{

  if (g_nand_drv.init == FALSE)
  {
    /* enable NAND clocks */
    cgu_clk_en_dis(CGU_SB_NANDFLASH_S0_CLK_ID, TRUE);
    cgu_clk_en_dis(CGU_SB_NANDFLASH_ECC_CLK_ID, TRUE);
    cgu_clk_en_dis(CGU_SB_NANDFLASH_NAND_CLK_ID, TRUE);
    cgu_clk_en_dis(CGU_SB_NANDFLASH_PCLK_ID, TRUE);
    /* needed for LPC315x series only */
    cgu_clk_en_dis(CGU_SB_NANDFLASH_AES_CLK_ID, TRUE);
    /* reset NAND controller */
    cgu_soft_reset_module(NANDFLASH_NAND_RESET_N_SOFT);
    cgu_soft_reset_module(NANDFLASH_ECC_RESET_N_SOFT);
    /* need for LPC315x series only */
    cgu_soft_reset_module(NANDFLASH_AES_RESET_N_SOFT);

    /* check NAND mux signals */
    SYS_REGS->mux_nand_mci_sel = 0;

    /* configure NAND flash based on the open_cfg structure*/
  }

  return (INT_32)&g_nand_drv;
}

/***********************************************************************
 *
 * Function: nand_close
 *
 * Purpose: Close the NAND
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the
 *     NAND, set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to NAND config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS nand_close(INT_32 devid)
{
  STATUS status = _ERROR;


  return status;
}

/***********************************************************************
 *
 * Function: nand_ioctl
 *
 * Purpose: NAND configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to NAND config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS nand_ioctl(INT_32 devid,
                  INT_32 cmd,
                  INT_32 arg)
{
  STATUS status = _ERROR;
  return status;
}

/***********************************************************************
 *
 * Function: nand_read
 *
 * Purpose: WDT read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to WDT descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_read(INT_32 devid,
                 void *buffer,
                 INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: nand_write
 *
 * Purpose: WDT write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to WDT descriptor
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_write(INT_32 devid,
                  void *buffer,
                  INT_32 n_bytes)
{
  return 0;
}
