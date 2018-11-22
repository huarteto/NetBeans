/***********************************************************************
 * $Id:: lh79524_i2s_driver.c 27 2007-08-31 19:35:06Z kevinw           $
 *
 * Project: LH79520 i2s driver
 *
 * Description:
 *     This file contains driver support for the i2s module on the
 *     LH79520
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

#include "lh79524_i2s_driver.h"

/***********************************************************************
 * i2s driver private data
 **********************************************************************/

/* i2s device configuration structure */
STATIC I2S_CFG_T i2scfg;

/***********************************************************************
 * i2s driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: i2s_open
 *
 * Purpose: Open the i2s controller for operation
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the i2s peripheral register set address,
 *     disable and reset the watchdog timer, and return a pointer to the
 *     i2s config structure to the caller.
 *
 * Parameters:
 *     ipbase: i2s descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a i2s controller config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 i2s_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;

    if ((i2scfg.init == FALSE) && ((I2S_REGS_T *) ipbase == I2S))
    {
        /* Device is valid and not previously initialized */
        i2scfg.init = TRUE;

        /* Save and return address of peripheral block */
        i2scfg.regptr = (I2S_REGS_T *) ipbase;

        /* Return pointer to i2s configuration structure */
        status = (INT_32) &i2scfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: i2s_close
 *
 * Purpose: Close the i2s operation
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
 *     devid: Pointer to i2s config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS i2s_close(INT_32 devid)
{
    I2S_REGS_T *i2s;
    I2S_CFG_T *i2scfgptr = (I2S_CFG_T *) devid;
    STATUS status = _ERROR;

    if (i2scfgptr->init == TRUE)
    {
        i2s = i2scfgptr->regptr;
        i2scfgptr->init = FALSE;
        status = _NO_ERROR;
    }
    else
    {
        status = _ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * Function: i2s_ioctl
 *
 * Purpose: i2s configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate watchdog timer
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to i2s config structure
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
STATUS i2s_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    I2S_REGS_T *i2s;
    I2S_CFG_T *i2scfgptr = (I2S_CFG_T *) devid;
    STATUS status = _ERROR;
    
    if (i2scfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        i2s = i2scfgptr->regptr;

        switch (cmd)
        {
            /* Enable i2s converter */
            case I2S_ENABLE_LOCK:   
                if (arg == 0)   		/* disable */
                {
                    i2s->ctrl &= ~I2S_CTRL_I2SEN;
                }
                else if (arg == 1)		/* enable */
                {
                    i2s->ctrl |= I2S_CTRL_I2SEN;
                }
                break;
                
            /* Enable loop back mode for SSP converter */
            case I2S_ENABLE_LOOPBACK:   
                if (arg == 0)   		/* normal operation */
                {
                    i2s->ctrl &= ~I2S_CTRL_I2SLBM;
                }
                else if (arg == 1)		/* SSPTX_I2S tie to SSPRX_I2S */
                {
                    i2s->ctrl |= I2S_CTRL_I2SLBM;
                }
                break;
                
            /* Invert WS status bit */
            case I2S_INVERT_WS:   
                if (arg == 0)   		/* no change on WS */
                {
                    i2s->ctrl &= ~I2S_CTRL_WSINV;
                }
                else if (arg == 1)		/* invert the WS */
                {
                    i2s->ctrl |= I2S_CTRL_WSINV;
                }
                break;
                
            /* WS delay applied */
            case I2S_WS_DELAY_APPLY:   
                if (arg == 0)   		/* no delay, WS one clock before MSB */
                {
                    i2s->ctrl &= ~I2S_CTRL_WSDEL;
                }
                else if (arg == 1)		/* one clock delay, WS transition with MSB */
                {
                    i2s->ctrl |= I2S_CTRL_WSDEL;
                }
                break;
                
            /* Master clock invert */
            case I2S_MASTER_CLK_INVERT:   
                if (arg == 0)   		/* no invert on SSPCLKOUT */
                {
                    i2s->ctrl &= ~I2S_CTRL_MCLKINV;
                }
                else if (arg == 1)		/* invert clock on SSPCLKOUT */
                {
                    i2s->ctrl |= I2S_CTRL_MCLKINV;
                }
                break;
                
            /* Receive overrun int mask */
            case I2S_RECEIVE_OVERRUN_INT_MASK:   
                if (arg == 0)   		/* no mask */
                {
                    i2s->stat &= ~I2S_IMSC_RORIM;
                }
                else if (arg == 1)		/* mask the int */
                {
                    i2s->stat |= I2S_IMSC_RORIM;
                }
                break;
                
            /* Receive timeout int mask */
            case I2S_RECEIVE_TIMEOUT_INT_MASK:   
                if (arg == 0)   		/* no mask */
                {
                    i2s->stat &= ~I2S_IMSC_RTIM;
                }
                else if (arg == 1)		/* mask the int */
                {
                    i2s->stat |= I2S_IMSC_RTIM;
                }
                break;
                
            /* Receive receive fifo int mask */
            case I2S_RECEIVE_FIFO_INT_MASK:   
                if (arg == 0)   		/* no mask */
                {
                    i2s->stat &= ~I2S_IMSC_RXIM;
                }
                else if (arg == 1)		/* mask the int */
                {
                    i2s->stat |= I2S_IMSC_RXIM;
                }
                break;
                
            /* transmit fifo int mask */
            case I2S_TRANSMIT_FIFO_INT_MASK:   
                if (arg == 0)   		/* no mask */
                {
                    i2s->stat &= ~I2S_IMSC_TXIM;
                }
                else if (arg == 1)		/* mask the int */
                {
                    i2s->stat |= I2S_IMSC_TXIM;
                }
                break;
                
            /* transmit underrun int mask */
            case I2S_TRANSMIT_UNDERRUN_INT_MASK:   
                if (arg == 0)   		/* no mask */
                {
                    i2s->stat &= ~I2S_IMSC_TXUEM;
                }
                else if (arg == 1)		/* mask the int */
                {
                    i2s->stat |= I2S_IMSC_TXUEM;
                }
                break;
                
            /* external codec protocol error int mask */
            case I2S_CODEC_ERROR_INT_MASK:   
                if (arg == 0)   		/* no mask */
                {
                    i2s->stat &= ~I2S_IMSC_ECPEM;
                }
                else if (arg == 1)		/* mask the int */
                {
                    i2s->stat |= I2S_IMSC_ECPEM;
                }
                break;
                
            /* ssp protocol error int mask */
            case I2S_SSP_PROTOCOL_ERROR_INT_MASK:   
                if (arg == 0)   		/* no mask */
                {
                    i2s->stat &= ~I2S_IMSC_SSPPEM;
                }
                else if (arg == 1)		/* mask the int */
                {
                    i2s->stat |= I2S_IMSC_SSPPEM;
                }
                break;
                
            /* clear transmit underrun interrupt */
            case I2S_CLEAR_INT_TX_UNDERRUN: /* clear the interrupt */
                i2s->icr |= I2S_ICR_TXUEC;
                break;
                
            /* clear external codec protocol error interrupt */
            case I2S_CLEAR_INT_CODEC_ERR: /* clear the interrupt */
                i2s->icr |= I2S_ICR_ECPEC;
                break;
                
            /* clear ssp protocol error interrupt */
            case I2S_CLEAR_INT_SSP_PROTOCOL_ERR: /* clear the interrupt */
                i2s->icr |= I2S_ICR_SSPPEC;
                break;
                
            /* Get the i2s status */
            case I2S_GET_STATUS:
                switch (arg)
                {
                    /* Get the I2S status register value */
                    case I2S_GET_STATUS_REG:
                    	status = i2s->stat;
                    	
                    /* Get the I2S raw interrupt status register value */
                    case I2S_GET_RAW_INT_REG:
                    	status = i2s->ris;
                        break;
                    
                    /* Get the I2S masked interrupt status register value */
                    case I2S_GET_MASKED_INT_REG:
                    	status = i2s->mis;
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
 * Function: i2s_read
 *
 * Purpose: i2s read function (stub only)
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
INT_32 i2s_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: i2s_write
 *
 * Purpose: i2s write function (stub only)
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
INT_32 i2s_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}
