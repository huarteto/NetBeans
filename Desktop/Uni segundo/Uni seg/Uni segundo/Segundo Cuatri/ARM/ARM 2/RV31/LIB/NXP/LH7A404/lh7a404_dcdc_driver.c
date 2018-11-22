/***********************************************************************
 * $Id:: lh7a404_dcdc_driver.c 14 2007-08-28 16:08:18Z kevinw          $
 *
 * Project: LH7A404 DCDC driver
 *
 * Description:
 *     This file contains driver support for the DCDC module on the
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

#include "lh7a404_dcdc_driver.h"

/***********************************************************************
 * DCDC private driver defines
 **********************************************************************/

#define DCDC0_DUTYH_MASK 0x0000000F /* Mask for DCDC0 duty cycles when
                                       nEXTPWR is high */
#define DCDC0_DUTYL_MASK 0x000000F0 /* Mask for DCDC0 duty cycles when
                                       nEXTPWR is low */
#define DCDC1_DUTYH_MASK 0x00000F00 /* Mask for DCDC1 duty cycles when
                                       nEXTPWR is high */
#define DCDC1_DUTYL_MASK 0x0000F000 /* Mask for DCDC1 duty cycles when
                                       nEXTPWR is low */
#define DCDC0_FREQH_MASK 0x00000007 /* Mask for DCDC0 clock prescaler
                                       when nEXTPWR is high */
#define DCDC0_FREQL_MASK 0x00000070 /* Mask for DCDC0 clock prescaler
                                       when nEXTPWR is low */
#define DCDC1_FREQH_MASK 0x00000700 /* Mask for DCDC1 clock prescaler
                                       when nEXTPWR is high */
#define DCDC1_FREQL_MASK 0x00007000 /* Mask for DCDC1 clock prescaler
                                       when nEXTPWR is low */

/***********************************************************************
 * DCDC private driver data and types
 **********************************************************************/

/* DCDC device configuration structure type */
typedef struct
{
    BOOL_32 init;        /* Device initialized flag */
    DCDC_REGS_T *regptr; /* Pointer to DCDC registers */
} DCDC_CFG_T;

/* DCDC device configuration structure */
STATIC DCDC_CFG_T dcdccfg;

/***********************************************************************
 * DCDC public driver functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: dcdc_open
 *
 * Purpose: Open the DCDC pulse width modulators
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the DCDC peripheral register set address,
 *     disable the DCDC modulator, and return a pointer to the DCDC
 *     config structure to the caller.
 *
 * Parameters:
 *     ipbase: DCDC modulator descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a DCDC modulator config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 dcdc_open(void *ipbase, INT_32 arg)
{
    UNS_32 tmp;
    INT_32 status = 0;

    if ((dcdccfg.init == FALSE) && ((DCDC_REGS_T *) ipbase == DCDC))
    {
        /* Device is valid and not previously initialized */
        dcdccfg.init = TRUE;

        /* Save and return address of peripheral block */
        dcdccfg.regptr = (DCDC_REGS_T *) ipbase;

        /* Make sure DCDC modulators are initially disabled */
        tmp = DCDC->pmpcon & ~(DCDC0_DUTYL_MASK | DCDC0_DUTYH_MASK |
            DCDC1_DUTYL_MASK | DCDC1_DUTYH_MASK);
        DCDC->pmpcon = (tmp | DCDC_PMP_DRV0_DTYHI(0) |
            DCDC_PMP_DRV0_DTYLO(0) | DCDC_PMP_DRV1_DTYHI(0) |
            DCDC_PMP_DRV1_DTYHI(0));

        /* Return pointer to DCDC configuration structure */
        status = (INT_32) &dcdccfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: dcdc_close
 *
 * Purpose: Close the DCDC pulse width modulators
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to FALSE,
 *     disable the DCDC modulators, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to DCDC modulator config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS dcdc_close(INT_32 devid)
{
    UNS_32 tmp;
    DCDC_CFG_T *dcdccfgptr = (DCDC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (dcdccfgptr->init == TRUE)
    {
        /* Disable DCDC modulators */
        tmp = DCDC->pmpcon & ~(DCDC0_DUTYL_MASK | DCDC0_DUTYH_MASK |
            DCDC1_DUTYL_MASK | DCDC1_DUTYH_MASK);
        dcdccfgptr->regptr->pmpcon = (tmp | DCDC_PMP_DRV0_DTYHI(0) |
            DCDC_PMP_DRV0_DTYLO(0) | DCDC_PMP_DRV1_DTYHI(0) |
            DCDC_PMP_DRV1_DTYHI(0));

        status = _NO_ERROR;
        dcdccfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: dcdc_ioctl
 *
 * Purpose: DCDC pulse width modulators configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate DCDC modulators
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to DCDC modulator config structure
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
STATUS dcdc_ioctl(INT_32 devid,
                  INT_32 cmd,
                  INT_32 arg)
{
    DCDC_REGS_T *dcdcregs;
    UNS_32 tmp;
    DCDC_CFG_T *dcdccfgptr = (DCDC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (dcdccfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        dcdcregs = dcdccfgptr->regptr;

        switch (cmd)
        {
            case DCDC1_SET_DUTYL:
                /* Set duty cycle for when nEXTPWR signal is low */
                tmp = dcdcregs->pmpcon & ~DCDC0_DUTYL_MASK;
                dcdcregs->pmpcon = (tmp | DCDC_PMP_DRV0_DTYLO(arg));
                break;

            case DCDC1_SET_DUTYH:
                /* Set duty cycle for when nEXTPWR signal is low */
                tmp = dcdcregs->pmpcon & ~DCDC0_DUTYH_MASK;
                dcdcregs->pmpcon = (tmp | DCDC_PMP_DRV0_DTYHI(arg));
                break;

            case DCDC2_SET_DUTYL:
                /* Set duty cycle for when nEXTPWR signal is low */
                tmp = dcdcregs->pmpcon & ~DCDC1_DUTYL_MASK;
                dcdcregs->pmpcon = (tmp | DCDC_PMP_DRV1_DTYLO(arg));
                break;

            case DCDC2_SET_DUTYH:
                /* Set duty cycle for when nEXTPWR signal is low */
                tmp = dcdcregs->pmpcon & ~DCDC1_DUTYH_MASK;
                dcdcregs->pmpcon = (tmp | DCDC_PMP_DRV1_DTYHI(arg));
                break;

            case DCDC1_SET_PSCL:
                /* Set clock prescaler when nEXTPWR signal is low */
                tmp = dcdcregs->pmpfreq & ~DCDC0_FREQL_MASK;
                dcdcregs->pmpfreq = (tmp | DCDC_FREQ_DRV0_PRELO(arg));
                break;

            case DCDC1_SET_PSCH:
                /* Set clock prescaler when nEXTPWR signal is high */
                tmp = dcdcregs->pmpfreq & ~DCDC0_FREQH_MASK;
                dcdcregs->pmpfreq = (tmp | DCDC_FREQ_DRV0_PREHI(arg));
                break;

            case DCDC2_SET_PSCL:
                /* Set clock prescaler when nEXTPWR signal is low */
                tmp = dcdcregs->pmpfreq & ~DCDC1_FREQL_MASK;
                dcdcregs->pmpfreq = (tmp | DCDC_FREQ_DRV1_PRELO(arg));
                break;

            case DCDC2_SET_PSCH:
                /* Set clock prescaler when nEXTPWR signal is high */
                tmp = dcdcregs->pmpfreq & ~DCDC1_FREQH_MASK;
                dcdcregs->pmpfreq = (tmp | DCDC_FREQ_DRV1_PREHI(arg));
                break;

            case DCDC_GET_STATUS:
                /* Get a DCDC status value */
                switch (arg)
                {
                    case DCDC1_DUTYL:
                        status = (dcdcregs->pmpcon &
                            ~DCDC0_DUTYL_MASK);
                        break;

                    case DCDC1_DUTYH:
                        status = ((dcdcregs->pmpcon &
                            ~DCDC0_DUTYH_MASK) >> 4);
                        break;

                    case DCDC2_DUTYL:
                        status = ((dcdcregs->pmpcon &
                            ~DCDC1_DUTYH_MASK) >> 8);
                        break;

                    case DCDC2_DUTYH:
                        status = ((dcdcregs->pmpcon &
                            ~DCDC1_DUTYH_MASK) >> 12);
                        break;

                    case DCDC1_PSCL:
                        status = (dcdcregs->pmpfreq &
                            ~DCDC0_FREQL_MASK);
                        break;

                    case DCDC1_PSCH:
                        status = ((dcdcregs->pmpfreq &
                            ~DCDC0_FREQH_MASK) >> 4);
                        break;

                    case DCDC2_PSCL:
                        status = ((dcdcregs->pmpfreq &
                            ~DCDC1_FREQL_MASK) >> 8);
                        break;

                    case DCDC2_PSCH:
                        status = ((dcdcregs->pmpfreq &
                            ~DCDC1_FREQH_MASK) >> 12);
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
 * Function: dcdc_read
 *
 * Purpose: DCDC pulse width modulators read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to DCDC modulator config structure
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
INT_32 dcdc_read(INT_32 devid,
                 void *buffer,
                 INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: dcdc_write
 *
 * Purpose: DCDC pulse width modulators write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to DCDC modulator config structure
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
INT_32 dcdc_write(INT_32 devid,
                  void *buffer,
                  INT_32 n_bytes)
{
    return 0;
}
