/***********************************************************************
 * $Id:: lh7a404_pwm_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 PWM driver
 *
 * Description:
 *     This file contains driver support for the PWM module on the
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

#include "lh7a404_csc_driver.h"
#include "lh7a404_pwm_driver.h"

/***********************************************************************
 * PWM private driver data and types
 **********************************************************************/

/* PWM device configuration structure type */
typedef struct
{
    BOOL_32 init;        /* Device initialized flag */
    PWM_REGS_T *regptr;  /* Pointer to PWM registers */
} PWM_CFG_T;

/* PWM device configuration structure */
STATIC PWM_CFG_T pwmcfg;

/***********************************************************************
 * PWM public driver functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: pwm_open
 *
 * Purpose: Open the pulse width modulators
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the PWM peripheral register set address,
 *     disable the PWMs, and return a pointer to the PWM config
 *     structure to the caller.
 *
 * Parameters:
 *     ipbase: PWM descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a PWM config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 pwm_open(void *ipbase,
                INT_32 arg)
{
    INT_32 idx;
    INT_32 status = 0;

    if ((pwmcfg.init == FALSE) && ((PWM_REGS_T *) ipbase == PWM))
    {
        /* Device is valid and not previously initialized */
        pwmcfg.init = TRUE;

        /* Save and return address of peripheral block */
        pwmcfg.regptr = (PWM_REGS_T *) ipbase;

        /* Make sure PWM is initially disabled */
        for (idx = 0; idx <=1; idx++)
        {
            /* Default state of PWMs is disabled, maximum clock and
               duty cycle, not sync'd */
            pwmcfg.regptr->pwmregs[idx].pwmenable = 0;
            pwmcfg.regptr->pwmregs[idx].pwmtcount = PWM_TC(0xFFFF);
            pwmcfg.regptr->pwmregs[idx].pwmdcycle = PWM_DC(0xFFFF);
            pwmcfg.regptr->pwmregs[idx].pwminvert = 0;
            pwmcfg.regptr->pwmregs[idx].pwmsync = PWM_NORMAL;
        }

        /* Return pointer to PWM configuration structure */
        status = (INT_32) &pwmcfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: pwm_close
 *
 * Purpose: Close the pulse width modulators
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to FALSE,
 *     disable the PWMs, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to PWM config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS pwm_close(INT_32 devid)
{
    INT_32 idx;
    PWM_CFG_T *pwmcfgptr = (PWM_CFG_T *) devid;
    STATUS status = _ERROR;

    if (pwmcfgptr->init == TRUE)
    {
        /* Disable PWMs */
        for (idx = 0; idx <=1; idx++)
        {
            pwmcfg.regptr->pwmregs[idx].pwmenable = 0;
        }

        status = _NO_ERROR;
        pwmcfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: pwm_ioctl
 *
 * Purpose: Pulse width modulators configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate PWM parameter.
 *
 * Parameters:
 *     devid: Pointer to PWM config structure
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
STATUS pwm_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    PWM_REGS_T *pwmregs;
    UNS_32 tmp;
    INT_32 idx = 0;
    PWM_CFG_T *pwmcfgptr = (PWM_CFG_T *) devid;
    STATUS status = _ERROR;

    if (pwmcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        pwmregs = pwmcfgptr->regptr;

        switch (cmd)
        {
            case PWM2_ENABLE:
                /* Enables or disables PWM2 */
                idx = 1;
            case PWM1_ENABLE:
                /* Enables or disables PWM1 */
                if (arg == 1)
                {
                    /* Enable the PWM */
                    pwmregs->pwmregs[idx].pwmenable = PWM_ENABLE;
                }
                else
                {
                    pwmregs->pwmregs[idx].pwmenable = 0;
                }
                break;

            case PWM2_INVERT:
                /* Enables or disables signal inversion on PWM2 */
                idx  = 1;
            case PWM1_INVERT:
                /* Enables or disables signal inversion on PWM1 */
                if (arg == 1)
                {
                    /* Enable inversion */
                    pwmregs->pwmregs[idx].pwminvert = PWM_INVERT;
                }
                else
                {
                    pwmregs->pwmregs[idx].pwmenable = 0;
                }
                break;

            case PWM2_SETTC:
                /* Set PWM2 terminal count */
                idx  = 1;
            case PWM1_SETTC:
                /* Set PWM1 terminal count */
                pwmregs->pwmregs[idx].pwmtcount = (UNS_32) arg;
                break;

            case PWM2_SETDT:
                /* Set PWM2 duty cycle */
                idx  = 1;
            case PWM1_SETDT:
                /* Set PWM1 duty cycle */
                pwmregs->pwmregs[idx].pwmdcycle = (UNS_32) arg;
                break;
                
            case PWM2_SETFREQ:
                /* Set PWM2 frequency */
                idx  = 1;
            case PWM1_SETFREQ:
                /* Set PWM1 frequency */
                /* Compute closest divider to get desired frequency */
                tmp = csc_compute_divider(CLOCK_MAINOSC, (UNS_32) arg);

                /* Set frequency */
                pwmregs->pwmregs[idx].pwmtcount = (tmp - 1);
                break;

            case PWM2_SETDUTY:
                /* Set PWM2 duty cycle */
                idx  = 1;
            case PWM1_SETDUTY:
                /* Set PWM1 duty cycle */

                /* Compute duty cycle value */
                tmp = pwmregs->pwmregs[idx].pwmtcount + 1;
                tmp = tmp * (UNS_32) arg;
                tmp = tmp / 100;
                pwmregs->pwmregs[idx].pwmdcycle = (tmp - 1);
                break;

            case PWM1_SET_SYNC:
                /* Enable or disable PWM1 sync */
                idx  = 1;
            case PWM2_SET_SYNC:
                /* Enable or disable PWM2 sync */
                if (arg == 1)
                {
                    /* Enable SYNC */
                    pwmregs->pwmregs[idx].pwmsync = PWM_SYNC;
                }
                else
                {
                    pwmregs->pwmregs[idx].pwmsync = PWM_NORMAL;
                }
                break;

            case PWM_GET_STATUS:
                /* Get a PWM status */
                switch (arg)
                {
                    case PWM2_ENABLED:
                        /* Returns 1 if PWM2 is enabled, otherwise 0 */
                        idx = 1;
                    case PWM1_ENABLED:
                        /* Returns 1 if PWM1 is enabled, otherwise 0 */
                        if ((pwmregs->pwmregs[idx].pwmenable &
                            PWM_ENABLE) != 0)
                        {
                            /* PWM is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* PWM is disabled */
                            status = 0;
                        }
                        break;

                    case PWM2_INVERTED:
                        /* Returns 1 if PWM2 is inverted, otherwise 0  */
                        idx = 1;
                    case PWM1_INVERTED:
                        /* Returns 1 if PWM1 is inverted, otherwise 0  */
                        if ((pwmregs->pwmregs[idx].pwminvert &
                            PWM_INVERT) != 0)
                        {
                            /* PWM is inverted */
                            status = 1;
                        }
                        else
                        {
                            /* PWM is not inverted */
                            status = 0;
                        }
                        break;

                    case PWM2_TC:
                        /* Returns PWM2 terminal count */
                        idx = 1;
                    case PWM1_TC:
                        /* Returns PWM1 terminal count */
                        status = (INT_32) pwmregs->pwmregs[idx].pwmtcount;
                        break;

                    case PWM2_DT:
                        /* Returns PWM2 duty cycle count */
                        idx = 1;
                    case PWM1_DT:
                        /* Returns PWM1 duty cycle count */
                        status = (INT_32) pwmregs->pwmregs[idx].pwmdcycle;
                        break;

                    case PWM2_SYNCD:
                        /* Returns 1 if PWM2 uses SYNC signal, otherwise
                           returns 0 */
                        idx = 1;
                    case PWM1_SYNCD:
                        /* Returns 1 if PWM1 uses SYNC signal, otherwise
                           returns 0 */
                        if ((pwmregs->pwmregs[idx].pwmsync &
                            PWM_SYNC) != 0)
                        {
                            /* PWM is inverted */
                            status = 1;
                        }
                        else
                        {
                            /* PWM is not inverted */
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
 * Function: pwm_read
 *
 * Purpose: Pulse width modulators read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to PWM config structure
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
INT_32 pwm_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: pwm_write
 *
 * Purpose: Pulse width modulators write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to PWM config structure
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
INT_32 pwm_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}
