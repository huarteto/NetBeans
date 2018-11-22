/***********************************************************************
 * $Id:: lh79524_wdt_driver.c 27 2007-08-31 19:35:06Z kevinw           $
 *
 * Project: LH79524 WDT driver
 *
 * Description:
 *     This file contains driver support for the WDT module on the
 *     LH79524
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
 *
 ***********************************************************************/

#include "lh79524_wdt_driver.h"

/***********************************************************************
 * WDT driver private data
 **********************************************************************/

/* WDT device configuration structure */
STATIC WDT_CFG_T wdtcfg;

/***********************************************************************
 * WDT driver public functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: wdt_open
 *
 * Purpose: Open the watchdog timer
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the WDT peripheral register set address,
 *     disable and reset the watchdog timer, and return a pointer to the
 *     WDT config structure to the caller.
 *
 * Parameters:
 *     ipbase: Watchdog timer descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a watchdog timer config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 wdt_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;

    if ((wdtcfg.init == FALSE) && ((WDT_REGS_T *) ipbase == WDTIMER))
    {
        /* Device is valid and not previously initialized */
        wdtcfg.init = TRUE;

        /* Save and return address of peripheral block */
        wdtcfg.regptr = (WDT_REGS_T *) ipbase;

        /* Default state of WDT is disabled, 2^16 PCLKs per tick,
           interrupt first disabled, freeze off, and initially reset */
        wdtcfg.regptr->wdctlr = 0x0;
        wdtcfg.regptr->wdcntr = WDT_WDCNTR;
        
        /* Reset the watchdog timer so it won't time out */
        wdtcfg.regptr->wdcntr = WDT_WDCNTR;
        
        /* Return pointer to WDT configuration structure */
        status = (INT_32) &wdtcfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: wdt_close
 *
 * Purpose: Close the watchdog timer
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, check the status of
 *     the freeze and enable bits. If they are set, then return
 *     SMA_CANT_STOP to the caller as the device can no longer be
 *     stopped. Otherwise, disable the watchdog timer, set init to
 *     FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to watchdog timer config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS wdt_close(INT_32 devid)
{
    WDT_REGS_T *wdtimer;
    WDT_CFG_T *wdtcfgptr = (WDT_CFG_T *) devid;
    STATUS status = _ERROR;

    if (wdtcfgptr->init == TRUE)
    {
        wdtimer = wdtcfgptr->regptr;
        if ((wdtimer->wdctlr & (WDT_CTRL_ENABLE | WDT_CTRL_FRZ_ENABLE)) ==
            (WDT_CTRL_ENABLE | WDT_CTRL_FRZ_ENABLE))
        {
            /* Watchdog timer is enabled and frozen, so it can't be
               stopped! */
            status = _ERROR;
        }
        else
        {
            status = _NO_ERROR;
            wdtcfgptr->init = FALSE;
            wdtimer->wdctlr = 0x0;
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: wdt_ioctl
 *
 * Purpose: Watchdog timer configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate watchdog timer
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to watchdog timer config structure
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
STATUS wdt_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    WDT_REGS_T *wdtimer;
    UNS_32 tmp;
    WDT_CFG_T *wdtcfgptr = (WDT_CFG_T *) devid;
    STATUS status = _ERROR;

    if (wdtcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        wdtimer = wdtcfgptr->regptr;

        switch (cmd)
        {
            case WDT_ENABLE:
                 /* Enable watchdog timer */
                 wdtimer->wdctlr |= WDT_CTRL_ENABLE;
                 /* Start the count down */
                 wdtcfg.regptr->wdcntr = WDT_WDCNTR;
                 break;
                 
            case WDT_DISABLE:
                 if ((wdtimer->wdctlr & WDT_CTRL_FRZ_ENABLE) != 0)
                 {
                     /* Freeze bit is set, watchdog timer cannot be
                        disabled */
                     status = _ERROR;
                 }
                 else
                 {
                     /* Disable watchdog timer if freeze bit is not
                        set */
                     wdtimer->wdctlr &= ~WDT_CTRL_ENABLE;
                 }
                 break;
            
            case WDT_FREEZE:
                /* Enable freeze bit and watchdog timer */
                 wdtimer->wdctlr |= (WDT_CTRL_ENABLE | WDT_CTRL_FRZ_ENABLE);
                 break;

            case WDT_RESET:
                /* Reset the watchdog timer so it won't time out */
                wdtimer->wdcntr = WDT_WDCNTR;
                break;

            case WDT_RESET_ON_TIMEOUT:
                /* Set a system reset will be generated when WDT time out */
                WDTIMER->wdctlr &= WDT_CTRL_RSP_RESET;
                break;
                
            case WDT_INT_ON_TIMEOUT:
                /* Set a interrupt will be set when first time-out occur */
                WDTIMER->wdctlr |= WDT_CTRL_RSP_INT;
                break;
                
            case WDT_SET_COUNT:
                /* Set watchdog timer count value */
                tmp = wdtimer->wdctlr & ~(_BIT(4)|_BIT(5)|_BIT(6)|_BIT(7));
                wdtimer->wdctlr = tmp | WDT_CTRL_TOP((INT_32) arg);
                break;

            case WDT_GET_STATUS:
                switch (arg)
                {
                    case WDT_GET_ENABLE:
                        if ((wdtimer->wdctlr & WDT_CTRL_ENABLE) != 0)
                        {
                            /* Watchdog timer is enabled */
                            status = TRUE;
                        }
                        else
                        {
                            /* Watchdog timer is disabled */
                            status = FALSE;
                        }
                        break;

                    case WDT_GET_FREEZE:
                        if ((wdtimer->wdctlr & WDT_CTRL_FRZ_ENABLE) != 0)
                        {
                            /* Freeze mode is enabled */
                            status = TRUE;
                        }
                        else
                        {
                            /* Freeze mode is disabled */
                            status = FALSE;
                        }
                        break;
                    
                    case WDT_GET_INT_FIRST:
                        if ((wdtimer->wdctlr & WDT_CTRL_RSP_INT) != 0)
                        {
                            /* WDT will cause a interrupt first */
                            status = TRUE;
                        }
                        else
                        {
                            /* WDT will cause reset anyway */
                            status = FALSE;
                        }
    
                    case WDT_GET_RESET_TRIGGERED:
                        if((WDTIMER->wdtstr & WDT_WD_NWDRES) == 0)
                        {
                            // WDT RESET has occured
                            status = TRUE;
                        }
                        else
                        {
                            // WDT RESET has not occured
                            status = FALSE;
                        }
                        break;

                    case WDT_GET_INT_TRIGGERED:
                        if((WDTIMER->wdtstr & WDT_WD_NWDINT) != 0)
                        {
                            // WDT INT has occured
                            status = TRUE;
                        }
                        else
                        {
                            // WDT INT has not occured
                            status = FALSE;
                        }
                        break;

                    case WDT_GET_COUNT:
                        /* Return count value */
                        status =
                            _SBF(24, WDTIMER->wdcnt3) | 
                            _SBF(16, WDTIMER->wdcnt2) | 
                            _SBF(8, WDTIMER->wdcnt1) | 
                            (WDTIMER->wdcnt0);
                        break;
                        
                    default:
                        /* Unsupported parameter */
                        status = _ERROR;
                        break;
                }
                break;

            default:
                /* Unsupported parameter */
                status = _ERROR;
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: wdt_read
 *
 * Purpose: WDT read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to watchdog timer config structure
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
INT_32 wdt_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: wdt_write
 *
 * Purpose: WDT write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to watchdog timer config structure
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
INT_32 wdt_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}
