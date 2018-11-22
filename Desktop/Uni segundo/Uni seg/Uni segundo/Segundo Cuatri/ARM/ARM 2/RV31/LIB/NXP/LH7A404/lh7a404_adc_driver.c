/***********************************************************************
 * $Id:: lh7a404_adc_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 ADC driver
 *
 * Description:
 *     This file contains driver support for the ADC module on the
 *     LH7A404
 *
 * Notes:
 *     The number of different configurations supported by the ADC
 *     is beyond the scope of this driver. This driver provides the
 *     following basic functions:
 *         Sequenced analog to digital conversions (polled, interrupt)
 *         Touchscreen conversions (polled, interrupt)
 *         Pendown state detect to touchscreen conversions (interrupt)
 *         Brownout interrupt handling (interrupt only)
 *         ADC status polling and general configuration
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

#include "lh7a404_adc_driver.h"
#include "lh7a404_csc_driver.h"

/***********************************************************************
 * ADC driver private data and types
 **********************************************************************/

/* Size of the ADC receive ring buffer */
#define ADC_RING_BUFSIZE 64

/* Function prototype used for polled and interrupt driven reads */
typedef INT_32 (*ADC_RFUNC_T) (void *, void *, INT_32);

/* ADC device configuration structure type */
typedef struct
{
    BOOL_32 init;       /* Device initialized flag */
    ADC_REGS_T *regptr; /* Pointer to ADC registers */
    UNS_16 rx[ADC_RING_BUFSIZE];  /* ADC data ring buffer */
    INT_32 rx_head;     /* ADC ring buffer head index */
    INT_32 rx_tail;     /* ADC ring buffer tail index */
    UNS_16 conv_count;  /* Number of conversions to perform */
    PFV bro_cbfunc;     /* Brown interrupt callback function */
    PFV pd_cbfunc;      /* Pendown interrupt completion callback
                           function */
    PFV eos_cbfunc;     /* End of sequence interrupt callback
                           function */
} ADC_CFG_T;

/* ADC device configuration structure */
STATIC ADC_CFG_T adccfg;

/***********************************************************************
 * ADC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: adc_default
 *
 * Purpose: Places the ADC interface and controller in a default state
 *
 * Processing:
 *     Disable ADC interrupts, set ADC power control to off (idle) with
 *     maximum clock divider and 1 conversion, set general configuration
 *     to continuous conversion mode. Set up each MUX input to maximum
 *     clock with default references. Set up idle high and low control
 *     words. Clear the ADC FIFO and clear any latched interrupts.
 *
 * Parameters:
 *     adcregs: Pointer to an ADC register set
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void adc_default(ADC_REGS_T *adcregs)
{
    INT_32 idx;

    /* Setup default state of ADC as idle, with all channels set up
       in a safe configuration, and all interrupts masked */
    /* Mask all interrupts */
    adcregs->adcie = 0;

    /* Setup mode as disabled, 1 conversion in a sequence, and
       maximum ADC clock divider */
    adcregs->adcpcr = (ADC_NOC_SET(1) | ADC_PWM_ALL_OFF |
        ADC_CLKSEL_DIV128);

    /* Set conversion mode to continuous with a single FIFO entry
       watermark level */
    adcregs->adcgencfg = (ADC_SSM_SSB | ADC_WM_SET(1));

    /* Set up each control bank register to use a linear analog
       input mapped array with maximum settling time */
    for (idx = 0; idx <= 15; idx++)
    {
        /* High control word */
        adcregs->adchicl[idx] = (ADC_SETTIME(0X1FF) |
            ADC_INPSEL(idx) | ADC_INM_GND | ADC_REF_VREFP);

        /* Low control word */
        adcregs->adclocl[idx] = ADC_REF_VREFN;
    }

    /* Setup idle mode high and low words using analog input 0
       as the default idle input */
    adcregs->adcidlehi = (ADC_SETTIME(0X1FF) | ADC_INP_AN0 |
        ADC_INM_GND | ADC_REF_VREFP);
    adcregs->adcidlelo = ADC_REF_VREFP;

    /* Make sure the FIFO is clear */
    while ((adcregs->adcfifosts & ADC_FIFO_EMPTY) == 0)
    {
        idx = (INT_32) adcregs->adcresults;
    }
    
    /* Clear pending ADC interrupts */
    adcregs->adcic = (ADC_EOS_CLR | ADC_PEN_CLR | ADC_BROWNOUT_CLR);
}

/***********************************************************************
 *
 * Function: adc_add_sequence_entry
 *
 * Purpose: Adds a new sample entry to the sequence list
 *
 * Processing:
 *     If the number of existing conversion sequence entries is greater
 *     than 16, then return _ERROR to the caller. Otherwise, generate
 *     the high and low sequence control words and place them into the
 *     the sequence register array indexed by conv_count. Set status to
 *     conv_count and increment conv_count by one. Return conv_count to
 *     the caller.
 *
 * Parameters:
 *     adc_ch_cfgptr: Pointer to a channel configuration structure
 *     adccfgptr:     Pointer to ADC config structure
 *
 * Outputs: None
 *
 * Returns: A key to the new sequence entry (0 to 15)
 *
 * Notes: None
 *
 **********************************************************************/
STATIC STATUS adc_add_sequence_entry(ADC_CH_CONV_T *adc_ch_cfgptr,
                              ADC_CFG_T *adccfgptr)
{
    UNS_32 tmphi, tmplo;
    STATUS status = _ERROR;

    /* Limit conversion count to 16 conversions */
    if (adccfgptr->conv_count < 16)
    {
        /* Build the high and low control words based on the passed
           channel configuration values */
        tmphi = (ADC_SETTIME(adc_ch_cfgptr->settle_clks) |
            ADC_INPSEL(adc_ch_cfgptr->inpc) |
            ADC_INNSEL(adc_ch_cfgptr->innc) |
            ADC_REFP_SEL(adc_ch_cfgptr->vrefp));

        /* Build and save the control bank low word register */
        tmplo = (ADC_REFN_SEL(adc_ch_cfgptr->vrefn) |
            adc_ch_cfgptr->bias);

        /* Write registers */
        adccfgptr->regptr->adchicl[adccfgptr->conv_count] = tmphi;
        adccfgptr->regptr->adclocl[adccfgptr->conv_count] = tmplo;

        /* Update the number of conversions in the sequence */
        tmphi = adccfgptr->regptr->adcpcr & ~ADC_NOC_MASK;
        adccfgptr->regptr->adcpcr = (tmphi | adccfgptr->conv_count);

        /* Save key return value */
        status = (STATUS) adccfgptr->conv_count;

        /* Increment conversion count value */
        adccfgptr->conv_count++;
    }

    return status;
}

/***********************************************************************
 *
 * Function: adc_chg_sequence_entry
 *
 * Purpose: Change an existing sequence entry
 *
 * Processing:
 *     If the sequence key (in the ADC_CH_CONV_T structure) is not
 *     within the limits of the converion list, return _ERROR to the
 *     caller. Otherwise, generate the new high and low configuration
 *     words for the channel indicated by the sequence key and update
 *     the registers in the ADC for that channel. Return _NO_ERROR to
 *     the caller.
 *
 * Parameters:
 *     adc_ch_cfgptr: Pointer to a channel configuration structure
 *     adccfgptr:     Pointer to ADC config structure
 *
 * Outputs: None
 *
 * Returns: _ERROR is the sequence key is invalid, otherwise _NO_ERROR.
 *
 * Notes: None
 *
 **********************************************************************/
STATIC STATUS adc_chg_sequence_entry(ADC_CH_CONV_T *adc_ch_cfgptr,
                              ADC_CFG_T *adccfgptr)
{
    UNS_32 tmphi, tmplo;
    STATUS status = _ERROR;

    /* Make sure the passed sequence entry is valid */
    if (adc_ch_cfgptr->chkey <= adccfgptr->conv_count)
    {
        /* Key is valid */
        
        /* Build the high and low control words based on the passed
           channel configuration values */
        tmphi = (ADC_SETTIME(adc_ch_cfgptr->settle_clks) |
            ADC_INPSEL(adc_ch_cfgptr->inpc) |
            ADC_INNSEL(adc_ch_cfgptr->innc) |
            ADC_REFP_SEL(adc_ch_cfgptr->vrefp));

        /* Build and save the control bank low word register */
        tmplo = (ADC_REFN_SEL(adc_ch_cfgptr->vrefn) |
            adc_ch_cfgptr->bias);

        /* Write registers */
        adccfgptr->regptr->adchicl[adc_ch_cfgptr->chkey] = tmphi;
        adccfgptr->regptr->adclocl[adc_ch_cfgptr->chkey] = tmplo;

        status = _NO_ERROR;
    }
    
    return status;
}

/***********************************************************************
 *
 * Function: adc_ring_fill
 *
 * Purpose: Move data from the ADC FIFO to the driver ring buffer
 *
 * Processing:
 *     While the ADC FIFO is not empty, copy an entry from the ADC
 *     FIFO into the ADC driver ring buffer. Increment the ring buffer
 *     head pointer. If it exceeds the ring buffer size, reset the
 *     head pointer to 0.
 *
 * Parameters:
 *     adccfgptr: Pointer to ADC config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void adc_ring_fill(ADC_CFG_T *adccfgptr)
{
    ADC_REGS_T *adcregsptr = (ADC_REGS_T *) adccfgptr->regptr;

    /* Continue stuffing the receive ring FIFO until the receive
       FIFO is empty */
    while ((adcregsptr->adcfifosts & ADC_FIFO_EMPTY) == 0)
    {
        adccfgptr->rx[adccfgptr->rx_head] =
            (UNS_16) adcregsptr->adcresults;

        /* Increment receive ring buffer head pointer */
        adccfgptr->rx_head++;
        if (adccfgptr->rx_head >= ADC_RING_BUFSIZE)
        {
            adccfgptr->rx_head = 0;
        }
    }
}

/***********************************************************************
 * ADC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: adc_open
 *
 * Purpose: Open the ADC
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *     set init to TRUE, save the ADC peripheral register set address,
 *     and initialize the ADC interface and controller to a default
 *     state by calling adc_default(), and return a pointer to the ADC
 *     config structure to the caller.
 *
 * Parameters:
 *     ipbase: ADC descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a ADC config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 adc_open(void *ipbase,
                INT_32 arg)
{
    INT_32 status = 0;

    if ((adccfg.init == FALSE) && ((ADC_REGS_T *) ipbase == ADC))
    {
        /* Device is valid and not previously initialized */
        adccfg.init = TRUE;

        /* Save and return address of peripheral block */
        adccfg.regptr = (ADC_REGS_T *) ipbase;

        /* Place ADC in a default state */
        adc_default(adccfg.regptr);

        /* Empty ring buffer and set conversion counter to 0 */
        adccfg.rx_head = adccfg.rx_tail = 0;
        adccfg.conv_count = 0;

        /* Clear default callback vector addresses for pendown and
           brownout interrupts */
        adccfg.bro_cbfunc = (PFV) NULL;
        adccfg.pd_cbfunc = (PFV) NULL;

        /* Return pointer to ADC configuration structure */
        status = (INT_32) &adccfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: adc_close
 *
 * Purpose: Close the ADC
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the ADC
 *     interface and controller by calling adc_default(), and return
 *     _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to ADC config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS adc_close(INT_32 devid)
{
    ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (adccfgptr->init == TRUE)
    {
        /* Restore ADC to a default state */
        adc_default(adccfg.regptr);

        status = _NO_ERROR;
        adccfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: adc_ioctl
 *
 * Purpose: ADC configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate ADC parameter.
 *
 * Parameters:
 *     devid: Pointer to ADC config structure
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
STATUS adc_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    ADC_REGS_T *adcregsptr;
    UNS_32 tmp, tmp1, tmp2;
    ADC_CH_CONV_T *chcfgptr;
    ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (adccfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        adcregsptr = adccfgptr->regptr;

        switch (cmd)
        {
            case ADC_SET_CLOCK:
                /* Set base speed of ADC clock in Hz */
                /* Try to find best ADC clock at or below desired
                   clock */
                tmp1 = 0;
                tmp2 = 1;
                while ((tmp1 < 7) &&
                    ((CLOCK_MAINOSC/8/tmp2) > (UNS_32) arg))
                {
                    tmp1++;
                    tmp2 = tmp2 * 2;
                }

                /* Best divider found, use it */
                tmp = adcregsptr->adcpcr & ~ADC_CLKSEL_MASK;
                adcregsptr->adcpcr = (tmp | ADC_CLKSEL_SET(tmp1));
                break;

            case ADC_CLEAR_FIFO:
                /* Clear the ADC sample FIFO */
                while ((adcregsptr->adcfifosts & ADC_FIFO_EMPTY) == 0)
                {
                    tmp = adcregsptr->adcresults;
                }
                break;

            case ADC_ENABLE_IREF:
                /* Enable or disable the internal reference buffer */
                if (arg == 1)
                {
                    adcregsptr->adcpcr |= ADC_REF_ENABLE;
                }
                else
                {
                    adcregsptr->adcpcr &= ~ADC_REF_ENABLE;
                }
                break;

            case ADC_CLEAR_INTS:
                /* Clear pending ADC interrupts */
                adcregsptr->adcic = ((UNS_32) arg &
                    (ADC_EOS_CLR | ADC_PEN_CLR | ADC_BROWNOUT_CLR));
                break;

            case ADC_INT_ENABLE:
                /* Enable ADC interrupts */
                adcregsptr->adcie |= ((UNS_32) arg &
                        (ADC_FIFO_OV_INT | ADC_WATERM_INT |
                        ADC_EOS_INT | ADC_PEN_INT)) | ADC_GLOBAL_INT;
                break;

            case ADC_INT_DISABLE:
                /* Disable ADC interrupts */
                adcregsptr->adcie &= ~((UNS_32) arg &
                    (ADC_FIFO_OV_INT | ADC_WATERM_INT |
                    ADC_EOS_INT | ADC_PEN_INT | ADC_GLOBAL_INT));
                if (adcregsptr->adcie == ADC_GLOBAL_INT)
                {
                    adcregsptr->adcie = 0;
                }                    
                break;

            case ADC_INSTALL_BRO:
                /* Install the brownout interrupt callback function */
                adccfgptr->bro_cbfunc = (PFV) arg;
                break;

            case ADC_INSTALL_PDC:
                /* Install the pendown interrupt callback function */
                adccfgptr->pd_cbfunc = (PFV) arg;
                break;

            case ADC_INSTALL_EOS:
                /* Install the end of sequence interrupt callback
                   function */
                adccfgptr->eos_cbfunc = (PFV) arg;
                break;
                
            case ADC_SET_IDLE_CH:
                /* Configure or reconfigure the ADC inputs used
                   during idle mode */
                chcfgptr = (ADC_CH_CONV_T *) arg;
                tmp = (ADC_SETTIME(chcfgptr->settle_clks) |
                    ADC_INPSEL(chcfgptr->inpc) |
                    ADC_INNSEL(chcfgptr->innc) |
                    ADC_REFP_SEL(chcfgptr->vrefp));
                adcregsptr->adcidlehi = tmp;

                /* Build and save the control bank low word register */
                tmp = (ADC_REFN_SEL(chcfgptr->vrefn) |
                    chcfgptr->bias);
                adcregsptr->adcidlelo = tmp;
                break;

            case ADC_SET_WMLEVEL:
                /* Set ADC FIFO watermark (interrupt trip) level */
                tmp = adcregsptr->adcgencfg & ~ADC_WM_MASK;
                adcregsptr->adcgencfg = (tmp |
                    ADC_WM_SET((UNS_32) arg));
                break;

            case ADC_ADD_CHANNEL:
                /* Add a conversion sequence entry */
                chcfgptr = (ADC_CH_CONV_T *) arg;
                status = adc_add_sequence_entry(chcfgptr, adccfgptr);
                break;

            case ADC_CHG_CHANNEL:
                /* Changes a conversion sequence entry with the passed
                   key argument */
                chcfgptr = (ADC_CH_CONV_T *) arg;
                status = adc_chg_sequence_entry(chcfgptr, adccfgptr);
                break;

            case ADC_START_SAMP:
                /* Starts an ADC sample */
                adcregsptr->adcgencfg |= ADC_SSB_START;
                break;

            case ADC_SET_STATE:
                /* Set sampling state */
                if (arg == ADC_MODE_RESET)
                {
                    /* Reset ADC controller and driver */
                    adc_default(adcregsptr);

                    /* Empty ring buffer and set conversion counter
                       to 0 */
                    adccfgptr->rx_head = adccfgptr->rx_tail = 0;
                    adccfgptr->conv_count = 0;
                }
                else if (arg == ADC_MODE_STOP)
                {
                    /* Stop the ADC controller */
                    tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
                    adcregsptr->adcpcr = (tmp | ADC_PWM_ALL_OFF);
                }
                else if (arg == ADC_SW_TRIGGERED)
                {
                    /* ADC mode is polled/sw triggered */
                    /* Disable and clear ADC interrupts */
                    adcregsptr->adcie &= ~((UNS_32) arg &
                        (ADC_FIFO_OV_INT | ADC_WATERM_INT |
                        ADC_EOS_INT | ADC_PEN_INT | ADC_GLOBAL_INT));
                    adcregsptr->adcic = ((UNS_32) arg &
                        (ADC_EOS_CLR | ADC_PEN_CLR | ADC_BROWNOUT_CLR));

                    /* Standby mode  */
                    tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
                    adcregsptr->adcpcr = (tmp | ADC_PWM_STANDBY);

                    /* Enable triggered mode  */
                    tmp = adcregsptr->adcgencfg & ~ADC_SSM_MASK;
                    adcregsptr->adcgencfg = (tmp | ADC_SSM_SSB);
                }
                else if (arg == ADC_PENDOWN_TRIGGERED)
                {
                    /* An ADC sequence is started on a pendown
                       detection event */
                    /* Standby mode  */
                    tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
                    adcregsptr->adcpcr = (tmp | ADC_PWM_STANDBY);

                    /* Enable pendown triggered mode  */
                    tmp = adcregsptr->adcgencfg & ~ADC_SSM_MASK;
                    adcregsptr->adcgencfg = (tmp | ADC_SSM_PEN);
                }
                else if (arg == ADC_PENDOWN_SW_TRIGGERED)
                {
                    /* An ADC sequence is started on a pendown
                       detection event or software request */
                    /* Standby mode  */
                    tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
                    adcregsptr->adcpcr = (tmp | ADC_PWM_STANDBY);

                    /* Enable pendown triggered mode  */
                    tmp = adcregsptr->adcgencfg & ~ADC_SSM_MASK;
                    adcregsptr->adcgencfg = (tmp | ADC_SSM_SSB_PEN);
                }
                else if (arg == ADC_CONTINUOUS)
                {
                    /* Place ADC in run mode */
                    tmp = adcregsptr->adcpcr & ~ADC_PWM_MASK;
                    adcregsptr->adcpcr = (tmp | ADC_PWM_RUN);

                    /* Continuous sampling */
                    tmp = adcregsptr->adcgencfg & ~ADC_SSM_MASK;
                    adcregsptr->adcgencfg = (tmp | ADC_SSM_CONT);
                }
                else
                {
                    /* Unknown mode */
                    status = SMA_BAD_PARAMS;
                }
                break;                

            case ADC_GET_STATUS:
                switch (arg)
                {
                    case ADC_RAWINT_ST:
                        /* Return raw interrupt statuses */
                        status = (INT_32) (adcregsptr->adcrawint &
                            (ADC_FIFO_OV_IRQ | ADC_WATERM_IRQ |
                            ADC_EOS_IRQ |ADC_PEN_IRQ |
                            ADC_BROWNOUT_IRQ));
                        break;

                    case ADC_INT_ST:
                        /* Return pending (enabled) interrupt
                           statuses */
                        status = (INT_32) (adcregsptr->adcmis &
                            (ADC_FIFO_OV_IRQ | ADC_WATERM_IRQ |
                            ADC_EOS_IRQ |ADC_PEN_IRQ |
                            ADC_BROWNOUT_IRQ));
                        break;

                    case ADC_CONV_COUNT:
                        /* Return the number of conversions in the
                           conversion count sequence */
                        status = adccfgptr->conv_count;
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
 * Function: adc_read_polled
 *
 * Purpose: Reads data directly from the ADC FIFO
 *
 * Processing:
 *     If the device is not initialized, return 0 to the caller.
 *     Otherwise, hile the number of bytes read is less than max_bytes
 *     and the ADC FIFO is not empty, read a FIFO entry from the ADC
 *     FIFO and place it into the passed user buffer. Increment bytes
 *     by 2. When the loop is exited, return the number of bytes read
 *     to the caller.
 *
 * Parameters:
 *     devid:     Pointer to an ADC configuration structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: The number of bytes read from the FIFO
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 adc_read_polled(INT_32 devid,
                       void *buffer,
                       INT_32 max_bytes)
{
    ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
    ADC_REGS_T *adcregptr = adccfgptr->regptr;
    UNS_16 *adcbuf = (UNS_16 *) buffer;
    INT_32 bytes = 0;

    if (adccfgptr->init == TRUE)
    {
        /* Grab all entries or until size equals max_bytes */
        while ((bytes < max_bytes) &&
            ((adcregptr->adcfifosts & ADC_FIFO_EMPTY) == 0))
        {
            *adcbuf = (UNS_16) adcregptr->adcresults;
            adcbuf++;
            bytes = bytes + 2;
        }
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: adc_read_ring
 *
 * Purpose: Reads data from the ADC ring buffer
 *
 * Processing:
 *     If the init flag for the ADC structure is FALSE, return 0 to
 *     the caller. Otherwise, save the state of the ADC interrupts and
 *     disable the ADC interrupts. Loop until max_bytes equals 0 or
 *     until the receive ring buffer is empty, whichever comes
 *     first. Read the data from the ring buffer  indexed by the tail
 *     pointer and place it into the user buffer. Increment the tail
 *     pointer and user buffer pointer. If the tail pointer exceeds the
 *     buffer size, set the tail pointer to 0. Increment bytes, and
 *     decrement max_bytes. Exit the loop based on the loop conditions,
 *     re-enable the receive interrupts, and return the number of bytes
 *     read to the caller.
 *
 * Parameters:
 *     devid:     Pointer to an ADC configuration structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: The number of bytes actually read from the ring buffer
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 adc_read_ring(INT_32 devid,
                     void *buffer,
                     INT_32 max_bytes)
{
    ADC_CFG_T *adccfgptr = (ADC_CFG_T *) devid;
    ADC_REGS_T *adcregs;
    UNS_32 tmp1;
    UNS_16 *data = (UNS_16 *) buffer;
    INT_32 bytes = 0;

    if (adccfgptr->init == TRUE)
    {
        adcregs = adccfgptr->regptr;

        /* Temporarily lock out ADC receive interrupt during this
           read so the ADC receive interrupt won't cause problems
           with the ring buffer index values */
        tmp1 = adcregs->adcie & (ADC_FIFO_OV_INT | ADC_WATERM_INT |
            ADC_EOS_INT | ADC_PEN_INT | ADC_GLOBAL_INT);
        adcregs->adcie &= ~(ADC_FIFO_OV_INT | ADC_WATERM_INT |
            ADC_EOS_INT | ADC_PEN_INT | ADC_GLOBAL_INT);

        /* Loop until receive ring buffer is empty or until max_bytes
           expires */
        while ((max_bytes > 0) &&
            (adccfgptr->rx_tail != adccfgptr->rx_head))
        {
            /* Read data from ring buffer into user buffer */
            *data = adccfgptr->rx[adccfgptr->rx_tail];
            data++;

            /* Update tail pointer */
            adccfgptr->rx_tail++;

            /* Make sure tail didn't overflow */
            if (adccfgptr->rx_tail >= ADC_RING_BUFSIZE)
            {
                adccfgptr->rx_tail = 0;
            }

            /* Increment data count and decrement buffer size count */
            bytes = bytes + 2;
            max_bytes = max_bytes - 2;
        }

        /* Re-enable ADC receive interrupt(s) */
        adcregs->adcie |= tmp1;
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: adc_write
 *
 * Purpose: ADC write function (stub only)
 *
 * Processing:
 *     Returns 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to ADC config structure
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
INT_32 adc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: adc_isr
 *
 * Purpose: ADC interrupt handler
 *
 * Processing:
 *     On an interrupt, read the (masked) interrupt status. Based on
 *     the status, handle the watermark or end of sequence interrupts
 *     by calling the adc_ring_fill() to move the data from the ADC
 *     FIFO to the ADC driver ring buffer. If the interrupt was due to
 *     an end of sequence and the end of sequence callback function
 *     exists, then call the user function. If the interrupt was due
 *     to a pendown event and the pendown callback function exists,
 *     then call the user function. If the interrupt was due to a
 *     brownout interrupt and the brownout callback function exists,
 *     then call the user function. 
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void adc_isr(void)
{
    UNS_32 int_status;

    /* Read interrupt status register */
    int_status = ADC->adcmis;

    /* Perform function based on interrupt status */
    if ((int_status & ADC_WATERM_INT) != 0)
    {
        /* FIFO needs servicing, called ring fill handler */
        adc_ring_fill(&adccfg);
    }

    if ((int_status & ADC_EOS_INT) != 0)
    {
        /* ADC end of sequence interrupt, move data from FIFO
           to ring buffer */
        adc_ring_fill(&adccfg);

        /* Call user function if it exists */
        if (adccfg.eos_cbfunc != (PFV) 0)
        {
            adccfg.eos_cbfunc();
        }
        
        /* Clear end of sequence interrupt */
        ADC->adcic = ADC_EOS_CLR;
    }

    if ((int_status & ADC_PEN_INT) != 0)
    {
        /* Clear end of pendown interrupt and call user function */
        ADC->adcic = ADC_PEN_CLR;

        /* Call user function if it exists */
        if (adccfg.pd_cbfunc != (PFV) 0)
        {
            adccfg.pd_cbfunc();
        }
    }

    if ((int_status & ADC_BROWNOUT_IRQ) != 0)
    {

        /* If the brownout interrupt callback function exists, call it */
        if (adccfg.bro_cbfunc != (PFV) 0)
        {
            adccfg.bro_cbfunc();
        }

        /* Brownout interrupt, clear it */
        ADC->adcic = ADC_BROWNOUT_CLR;
    }
}

/***********************************************************************
 *
 * Function: adc_strip_data
 *
 * Purpose: Strip out and justify analog data from a sample
 *
 * Processing:
 *     The data that is returned from the adc_read() function returns a
 *     value that has data and the analog input number mixed into it.
 *     This function strips off the analog input number and returns only
 *     the converted 10-bit data value right justified into a 16-bit
 *     field.
 *
 * Parameters:
 *     data : The raw ADC sample to extract the data from
 *
 * Outputs: None
 *
 * Returns: The right justified analog data value
 *
 * Notes: None
 *
 **********************************************************************/
UNS_16 adc_strip_data(UNS_16 data)
{
    return ADC_READ_OUTPUT(data);
}

/***********************************************************************
 *
 * Function: adc_strip_input
 *
 * Purpose: Strip out the analog input channel from a sample
 *
 * Processing:
 *     The data that is returned from the adc_read() function returns a
 *     value that has data and the analog input number mixed into it.
 *     This function strips off the converted data and returns only the
 *     analog input channel number.
 *
 * Parameters:
 *     data : The raw ADC sample to extract the data from
 *
 * Outputs: None
 *
 * Returns: The right justified analog data value
 *

 * Notes: None
 *
 **********************************************************************/
UNS_16 adc_strip_input(UNS_16 data)
{
    return (data & ADC_CBANKTAG);
}
