/***********************************************************************
 * $Id:: lh7a404_wdt_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 WDT driver
 *
 * Description:
 *     This file contains driver support for the WDT module on the
 *     LH7A404
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
 **********************************************************************/

#include "lh7a404_wdt_driver.h"

/***********************************************************************
 * WDT driver private data and types
 **********************************************************************/

/* WDT device configuration structure type */
typedef struct
{
    BOOL_32 init;       /* Device initialized flag */
    WDT_REGS_T *regptr; /* Pointer to WDT registers */
} WDT_CFG_T;

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
        if ((wdtimer->wdctlr & (WDT_CTRL_ENABLE | WDT_CTRL_FRZ)) ==
            (WDT_CTRL_ENABLE | WDT_CTRL_FRZ))
        {
            /* Watchdog timer is enabled and frozen, so it can't be
               stopped! */
            status = SMA_CANT_STOP;
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
                 if (arg == 1)
                 {
                     /* Enable watchdog timer */
                     wdtimer->wdctlr |= WDT_CTRL_ENABLE;
                 }
                 else if ((wdtimer->wdctlr & WDT_CTRL_FRZ) != 0)
                 {
                     /* Freeze bit is set, watchdog timer cannot be
                        disabled */
                     status = SMA_CANT_STOP;
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
                 wdtimer->wdctlr |= (WDT_CTRL_ENABLE | WDT_CTRL_FRZ);
                 break;

            case WDT_RESET:
                /* Reset the watchdog timer so it won't time out */
                wdtimer->wdcntr = WDT_WDCNTR;
                break;

            case WDT_INT_FIRST:
                 if (arg == 1)
                 {
                     /* Enable FIQ interrupt first mode */
                     wdtimer->wdctlr |= WDT_CTRL_RSP_FIQ;
                 }
                 else
                 {
                     /* Disable FIQ interrupt first mode */
                     wdtimer->wdctlr &= ~WDT_CTRL_RSP_FIQ;
                 }
                 break;

            case WDT_COUNT:
                /* Set watchdog timer count value */
                tmp = wdtimer->wdctlr & ~WDT_TO_MASK;
                wdtimer->wdctlr = tmp | WDT_CTRL_TOP((INT_32) arg);
                break;

            case WDT_GET_STATUS:
                switch (arg)
                {
                    case WDT_ENABLE_ST:
                        if ((wdtimer->wdctlr & WDT_CTRL_ENABLE) != 0)
                        {
                            /* Watchdog timer is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* Watchdog timer is disabled */
                            status = 0;
                        }
                        break;

                    case WDT_FREEZE_ST:
                        if ((wdtimer->wdctlr & WDT_CTRL_FRZ) != 0)
                        {
                            /* Freeze mode is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* Freeze mode is disabled */
                            status = 0;
                        }
                        break;

                    case WDT_RESET_ST:
                        if ((wdtimer->wdtstat & WDT_WD_NWDRES) != 0)
                        {
                            /* A system reset due to the watchdog timer
                               has not occurred */
                            status = 1;
                        }
                        else
                        {
                            /* A system reset due to the watchdog timer
                               has occurred */
                            status = 0;
                        }
                        break;

                    case WDT_INT_FIRST_ST:
                        if ((wdtimer->wdctlr & WDT_CTRL_RSP_FIQ) != 0)
                        {
                            /* FIQ interrupt first mode is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* FIQ interrupt first mode is disabled */
                            status = 0;
                        }
                        break;

                    case WDT_COUNT_ST:
                        /* Return count value */
                        status = (wdtimer->wdctlr & WDT_TO_MASK) >> 4;
                        break;

                    case WDT_FIQ_ST:
                        if ((wdtimer->wdtstat & WDT_WD_NWDFIQ) != 0)
                        {
                            /* An FIQ is pending */
                            status = 1;
                        }
                        else
                        {
                            /* An FIQ is not pending */
                            status = 0;
                        }
                        break;

                    default:
                        /* Unsupported parameter */
                        status = SMA_BAD_PARAMS;
                        break;
                }
                break;

            default:
                /* Unsupported parameter */
                status = SMA_BAD_PARAMS;
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
