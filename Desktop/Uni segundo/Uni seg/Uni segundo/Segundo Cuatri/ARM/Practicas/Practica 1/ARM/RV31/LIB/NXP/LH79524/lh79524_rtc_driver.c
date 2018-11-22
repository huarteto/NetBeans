/***********************************************************************
 * $Id:: lh79524_rtc_driver.c 27 2007-08-31 19:35:06Z kevinw           $
 *
 * Project: LH79524 RTC real time clock driver
 *
 * Description:
 *     This file contains driver support for the RTC module on the
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
 **********************************************************************/

#include "lh79524_rcpc.h"
#include "lh79524_rtc_driver.h"

/***********************************************************************
 * RTC driver private data
 **********************************************************************/

/* RTC device configuration structure */
STATIC RTC_CFG_T rtccfg;

/***********************************************************************
 * RTC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: rtc_open
 *
 * Purpose: Open the RTC
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the RTC peripheral register set address,
 *     disable and reset the match interrupt, and return a pointer to
 *     the RTC config structure to the caller.
 *
 * Parameters:
 *     ipbase: Real Time clock descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a Real Time clock config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 rtc_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;

    if ((rtccfg.init == FALSE) && ((RTC_REGS_T *) ipbase == RTC))
    {
        /* Device is valid and not previously initialized */
        rtccfg.init = TRUE;

        /* Save and return address of peripheral block */
        rtccfg.regptr = (RTC_REGS_T *) ipbase;

        /* Enable RTC clock in RCPC */
        RCPC->periphclkctrl0 &= ~RCPC_CLKCTRL0_RTC_DISABLE;
        
        /* Disable and clear match interrupt */
        rtccfg.regptr->cr = 0x00000000;
        rtccfg.regptr->icr = 0x00000000;

        /* Return pointer to RTC configuration structure */
        status = (INT_32) &rtccfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: rtc_close
 *
 * Purpose: Close the RTC
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to FALSE
 *     and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to Real time clock config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS rtc_close(INT_32 devid)
{
    RTC_CFG_T *rtccfgptr = (RTC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (rtccfgptr->init == TRUE)
    {
        /* Disable RTC clock in RCPC */
        RCPC->periphclkctrl0 |= RCPC_CLKCTRL0_RTC_DISABLE;
        
        /* 'Uninitialize' device */
        rtccfgptr->init = FALSE;
        status = _NO_ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * Function: rtc_ioctl
 *
 * Purpose: RTC configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate real time clock
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to Real time clock config structure
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
STATUS rtc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    RTC_REGS_T *rtcclock;
    RTC_CFG_T *rtccfgptr = (RTC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (rtccfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        rtcclock = rtccfgptr->regptr;

        switch (cmd)
        {
            case RTC_START:
                // Start the RTC with initial value specified
                // it could be 0x0 for RTC counter start to count from 0
                rtcclock->lr = (UNS_32) arg;
                rtcclock->cr |= RTC_CR_START;
                break;

            case RTC_SET_TIME_SECOND:
                /* Set RTC match value */
                rtcclock->mr = (UNS_32) (arg);
                break;

            case RTC_ENABLE_MATCH_INT:
                /* Enable RTC match interrupt */
                rtcclock->imsc |= RTC_IMSC_MASKED;
                break;

            case RTC_DISABLE_MATCH_INT:
                /* Disable RTC match interrupt */
                rtcclock->imsc &= ~RTC_IMSC_MASKED;
                break;
                
            case RTC_CLEAR_MATCH_INT:
                /* Clear the RTC match interrupt */
                rtcclock->icr |= RTC_ICR_CLEAR;
                break;

            case RTC_GET_STATUS:
                /* Return an RTC status */
                switch (arg)
                {
                    case RTC_GET_LOAD:
                        /* Get RTC clock value */
                         status = (INT_32) rtcclock->lr;
                         break;

                    case RTC_GET_CURRENT:
                        /* Get RTC current clock value */
                         status = (INT_32) rtcclock->dr;
                         break;

                    case RTC_GET_MATCH:
                        /* Get RTC match value */
                         status = (INT_32) rtcclock->mr;
                         break;

                    case GET_INT_PENDING:
                        if ((rtcclock->mis & RTC_MIS_INT) != 0)
                        {
                            /* RTC match interrupt is pending */
                            status = TRUE;
                        }
                        else
                        {
                            /* RTC match interrupt is not pending */
                            status = FALSE;
                        }
                        break;

                    default:
                        /* Unsupported parameter */
                        status = _ERROR;
                }
                break;

            default:
                /* Unsupported parameter */
                status = _NO_ERROR;
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: rtc_read
 *
 * Purpose: RTC read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to Real time clock config structure
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
INT_32 rtc_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return _ERROR;
}

/***********************************************************************
 *
 * Function: rtc_write
 *
 * Purpose: RTC write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to Real time clock config structure
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
INT_32 rtc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return _ERROR;
}
