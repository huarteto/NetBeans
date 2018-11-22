/***********************************************************************
 * $Id:: lh7a404_rtc_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 RTC driver
 *
 * Description:
 *     This file contains driver support for the RTC module on the
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

#include "lh7a404_rtc_driver.h"

/***********************************************************************
 * RTC driver private data and types
 **********************************************************************/

/* RTC device configuration structure type */
typedef struct
{
    BOOL_32 init;       /* Device initialized flag */
    RTC_REGS_T *regptr; /* Pointer to RTC registers */
} RTC_CFG_T;

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

        /* Disable and clear match interrupt */
        rtccfg.regptr->cr = 0x00000000;
        rtccfg.regptr->stat_eoi = 0x00000000;

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
            case RTC_SET_CLOCK:
                /* Set RTC clock value */
                 rtcclock->clr = (UNS_32) arg;
                 break;

            case RTC_SET_MATCH:
                /* Set RTC match value */
                 rtcclock->mr = (UNS_32) arg;
                 break;

            case RTC_ENABLE_MATCH_INT:
                if (arg == 1)
                {
                    /* Enable RTC match interrupt */
                    rtcclock->cr = RTC_CR_MIE;
                }
                else
                {
                    /* Disable RTC match interrupt */
                    rtcclock->cr = 0x00000000;
                }
                break;

            case RTC_CLEAR_MATCH_INT:
                /* Clear the RTC match interrupt */
                rtcclock->stat_eoi = 0x00000000;
                break;

            case RTC_GET_STATUS:
                /* Return an RTC status */
                switch (arg)
                {
                    case RTC_LOAD:
                        /* Get RTC clock value */
                         status = (INT_32) rtcclock->clr;
                         break;

                    case RTC_CURRENT:
                        /* Get RTC current clock value */
                         status = (INT_32) rtcclock->dr;
                         break;

                    case RTC_MATCH:
                        /* Get RTC match value */
                         status = (INT_32) rtcclock->mr;
                         break;

                    case RTC_INT_PENDING_ST:
                        if ((rtcclock->stat_eoi & RTC_STAT_INTR) != 0)
                        {
                            /* RTC match interrupt is pending */
                            status = 1;
                        }
                        else
                        {
                            /* RTC match interrupt is not pending */
                            status = 0;
                        }
                        break;

                    default:
                        /* Unsupported parameter */
                        status = SMA_BAD_PARAMS;
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
    return 0;
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
    return 0;
}
