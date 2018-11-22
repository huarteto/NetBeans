/***********************************************************************
 * $Id:: lh79524_ssp_driver.c 27 2007-08-31 19:35:06Z kevinw           $
 *
 * Project: LH79520 SSP driver
 *
 * Description:
 *     This file contains driver support for the SSP module on the
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

#include <assert.h>
#include "lh79524_rcpc.h"
#include "lh79524_iocon.h"
#include "lh79524_dmac.h"
#include "lh79524_vic.h"
#include "lh79524_gpio.h"
//#include "lh79524_mmu_driver.h"
#include "lh79524_ssp_driver.h"

#include "sdk79524_board.h"

/***********************************************************************
 * SSP driver private data
 **********************************************************************/

STATIC INT_32 ssp_set_speed(UNS_32 requested_bits_per_second);
STATIC INT_32 ssp_get_speed(void);

STATIC INT_32 hclk_freq;

/* SSP device configuration structure */
STATIC SSP_CFG_T sspcfg;

#define SSPFRM_GPIO_BIT _BIT(2)


/**********************************************************************
*
* Function: ssp_set_speed
*
* Purpose:
*  return the SSP receive timeout in microseconds
*
* Processing:
*  If the requested_bits_per_second is too fast, set the bit rate
*  as fast as possible.
*  If the requested_bits_per_second is too slow, set the bit rate as
*  slow as possible.
*  If the requested_bits_per_second is in range, set the RCPC
*  SSP clock prescaler register, SSP prescaler, and SSP divider
*  to obtain the clock as close as possible.
*
* Parameters: 
*  requested_bits_per_second: The desired bits per second
*
* Outputs: None
*
* Returns: 
*  the actual bits per second obtained or 0 if the requested bits
*  per second is not obtainable.
*
* Notes: 
*  The ssp_init_spi_mode, ssp_init_uwire_mode, or ssp_init_ti_mode
*  must be called first in order for this function to work.
*  RCPC write lock bit need to be cleared using
*   rcpc_reg_write_unlock() function call before using this function
*
*   set serial ssp serial clock rate. The bit rate is 
*   fSSPCLK + (CPSDVSR * (1+SCR)), where CPSDVSR is an
*   even value from 2 to 254 and SCR is a value from 0-255) 
*
**********************************************************************/
STATIC INT_32 ssp_set_speed(UNS_32 requested_bits_per_second)
{
    INT_32 ssp_prescale;
    INT_32 ssp_divider;
    INT_32 rcpc_prescale;
    INT_32 new_prescale;
    INT_32 new_divider;
    INT_32 quotient;
    INT_32 delta1;
    INT_32 delta2;  
    INT_32 min_error;
    INT_32 new_error;

    /* get the dividers as close to the requested freq as possible */
    if (requested_bits_per_second >= MAX_SSP_FREQ)
    {
        /* requested clock frequency is too fast. Set to max freq */
        ssp_prescale = SSP_PRESCALE_MIN;
        ssp_divider = 1;
        rcpc_prescale = 1;
    }
    else
    {
        /* compute the required divider as close as possible */
        quotient = hclk_freq / requested_bits_per_second;
        assert(quotient > 0); 

        /* round the quotient */
        delta1 = requested_bits_per_second - (hclk_freq / quotient );
        if (delta1 < 0)
            delta1 = - delta1;
        delta2 = requested_bits_per_second 
                 - (hclk_freq / (quotient + 1) );
        if (delta2 < 0)
            delta2 = - delta2;
        if (delta1 > delta2)
            quotient++;

        if (quotient >= (SSP_PRESCALE_MAX 
                         * RCPC_SSP_PRESCALE_MAX 
                         * SSP_DIVIDER_MAX) )
        {
            /* 
            Then requested clock frequency is <= minimum possible.
            Make it as slow as possible.
            */
            rcpc_prescale = RCPC_SSP_PRESCALE_MAX;
            ssp_prescale = SSP_PRESCALE_MAX;
            ssp_divider = SSP_DIVIDER_MAX;
        }
        else
        {
            /* 
            The computed quotient is in range.
            quotient is the target clock divide frequency; 
            get as close as possible 
            */

            rcpc_prescale = 1;
            /* 
            try to reduce power by using RCPC prescaler
            Note that the ssp prescaler minimum is two
            so can only prescale and maintain accuracy
            if quotient is divisble by 4.
            */           
            while ( ((quotient & 0x3) == 0) 
                    && (rcpc_prescale < RCPC_SSP_PRESCALE_MAX) )
            {
                quotient >>= 1;
                rcpc_prescale <<= 1;
            }

            /* 
            Make sure the requested frequency is within range
            of the SPP's prescaler and divider.
            Hopefully, this loop never executes. If it does,
            accuracy suffers.
            */
            while (quotient > (SSP_PRESCALE_MAX * SSP_DIVIDER_MAX) )
            {
                rcpc_prescale <<= 1;
                quotient >>= 1;
            }

            /* 
            factor the quotient into the divider and prescaler combo
            that minimizes the error in the quotient by exhaustively
            searching all legal ssp prescaler values.
            */
            ssp_prescale = SSP_PRESCALE_MIN;
            ssp_divider = (quotient / ssp_prescale);
            ssp_divider = (ssp_divider > SSP_DIVIDER_MAX) ? 
                                        SSP_DIVIDER_MAX : 
                                        ssp_divider;
            min_error = quotient - (ssp_divider * ssp_prescale);
            min_error = (min_error < 0) ? -min_error : min_error;
            for (new_prescale = SSP_PRESCALE_MIN + 2; 
                 new_prescale < SSP_PRESCALE_MAX;
                 new_prescale += 2)
            {
                new_divider = (quotient / new_prescale);
                new_divider = (new_divider > SSP_DIVIDER_MAX) ? 
                                        SSP_DIVIDER_MAX : 
                                        new_divider;
                new_error = quotient - (new_divider * new_prescale);
                new_error = (new_error < 0) ? -new_error : new_error;
                if (new_error < min_error)
                {
                    min_error = new_error;
                    ssp_prescale = new_prescale;
                    ssp_divider = new_divider;
                }
            }

        } /* end if quotient is greater than max */

    } /* end if requested frequency is too large */

    /* set up registers */
    RCPC->sspclkprescale = rcpc_prescale >> 1;

    SSP->cpsr = ssp_prescale;

    SSP->ctrl0 &= 0xff; /* clear old divider value */
    SSP->ctrl0 |= SSP_CR0_SCR(ssp_divider - 1);
    return ssp_get_speed();
}

/**********************************************************************
*
* Function: ssp_get_speed
*
* Purpose:
*  return the SSP speed in bits per second.
*
* Processing:
*  
*
* Parameters: 
*  time_in_us: The timeout interval in microseconds.
*
* Outputs: None
*
* Returns: 
*  Return the SSP bits per second
*
* Notes: None
*
**********************************************************************/
STATIC INT_32 ssp_get_speed(void)
{
    INT_32 rcpc_prescale;
    INT_32 ssp_prescale=1;
    INT_32 ssp_divider;
    
   
    if (ssp_prescale == 0)
       return 0;

    rcpc_prescale = RCPC->sspclkprescale;
    if (rcpc_prescale == 0)
       rcpc_prescale = 1;
    else
       rcpc_prescale <<= 1;
    ssp_prescale = SSP->cpsr;
    ssp_divider = (SSP->ctrl0 & _SBF(8,_BITMASK(8) ) ) >> 8;

    return hclk_freq / (rcpc_prescale 
                        * (ssp_prescale) 
                        * (ssp_divider + 1) );
}

/***********************************************************************
 * SSP driver public functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: ssp_open
 *
 * Purpose: Open the SSP controller
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *      return a pointer to the SSP config structure to the caller.
 *
 * Parameters:
 *     ipbase: SSP descriptor device address
 *
 * Outputs: None
 *
 * Returns: The pointer to a SSP config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;
    INT_32 temp1;

    if ((sspcfg.init == FALSE) && ((SSP_REGS_T *) ipbase == SSP))
    {
        /* Device is valid and not previously initialized */
        sspcfg.init = TRUE;

        /* Save and return address of peripheral block */
        sspcfg.regptr = (SSP_REGS_T *) ipbase;

        /* Return pointer to SSP configuration structure */
        status = (INT_32) &sspcfg;
        
        /* disable SSP DMA interrupts and transfer requests */
        /* set control registers to their reset defaults */
        SSP->ctrl1 = 0; 
        SSP->ctrl0 = 0;
        /* clear any SSP interrupts */
        SSP->icr |= SSP_ICR_RORIC | SSP_ICR_RTIC;

        RCPC->pclksel1 &= ~RCPC_PCLKSEL1_SSP_SCOC; /* source is hclk */
        RCPC->sspclkprescale = 0; /* as fast as possible */
        RCPC->periphclkctrl1 &= ~RCPC_CLKCTRL1_SSP_DISABLE; /* enable ssp clock */
        SSP->cpsr = 2;  /* this is the minimum legal value */
        /* 
        flush the FIFOs--SSP toggles the data out, clock, frame and enable
        signals at the GPIO MUX but not at the pins because the SSP pins
        are switched to alternate functions (GPIO or serial port)
        */
        SSP->ctrl1 |= SSP_CR1_SSE;
        /* flush transmit fifo */
        /* while if ssp transmit fifo is not empty */
        while ( (SSP->sr & SSP_SR_TFE) == 0);

        /* flush the receive fifo */
        /* while if ssp receive fifo is not empty */
        while ((SSP->sr & SSP_SR_RNE) != 0)
            temp1 = SSP->dr;
        
        /* clear any receive overruns */
        SSP->icr |= SSP_ICR_RORIC | SSP_ICR_RTIC;
        /* disable the SSP again */
        SSP->ctrl1 &= ~SSP_CR1_SSE;
        
        /* mask all the ssp interrupts */
    	SSP->imsc &= ~(SSP_IMSC_RORIM | SSP_IMSC_RTIM | SSP_IMSC_RXIM | SSP_IMSC_TXIM);
    
        /* set mux pin and make GPIO to be SSP pin */
        temp1 = IOCON->mux_ctl_5;
        temp1 &= 0xFF00;
		temp1 |= IOCON_MUX5_SSPTX_I2STXD |
				IOCON_MUX5_SSPRX_I2STXD |
        		IOCON_MUX5_SSPCLK_I2SLCK |
        		IOCON_MUX5_SSPFRM_I2SWS;
        IOCON->mux_ctl_5 = temp1;
        /* SSP is disabled on open */
        
        /* Get system pll value */
        temp1 = SDK79524_XTAL_IN * (RCPC->systempllctrl & 0x3f);
        temp1 = temp1 / ((RCPC->systempllctrl>>6) & 0x3f);
        hclk_freq = temp1/2 * RCPC->hclkprescale;
    }

    return status;
}

/***********************************************************************
 *
 * Function: ssp_close
 *
 * Purpose: Close the SSP controller
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. return
 *     _ERROR to the caller as the device can no longer be
 *     stopped. Otherwise, disable the SSP, set init to
 *     FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to SSP config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS ssp_close(INT_32 devid)
{
    SSP_CFG_T *sspcfgptr = (SSP_CFG_T *) devid;
    STATUS status = _ERROR;
    UNS_32 temp1;

    if (sspcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        sspcfgptr->init = FALSE;
        
        /* disable ssp clock */
        RCPC->periphclkctrl1 |= RCPC_CLKCTRL1_SSP_DISABLE; 

        /* disable SSP DMA interrupts and transfer requests */
        /* set control registers to their reset defaults */
        SSP->ctrl1 = 0; 
        SSP->ctrl0 = 0;
        /* clear any receive overruns */
        SSP->icr |= SSP_ICR_RORIC | SSP_ICR_RTIC;
   
        /* disable the ssp DMA streams */
        DMAC->stream0.max = 0;
        DMAC->stream0.ctrl = 0;
        DMAC->stream1.max = 0;
        DMAC->stream1.ctrl = 0;
        /* clear any previous SSP DMA completions */
        DMAC->clear = DMAC_EOT0 | DMAC_EOT1;
        /* set mux register and make SSP pin to be GPIO */
        /* switch all muxed I/O away from the SSP */
        temp1 = IOCON->mux_ctl_5;
        temp1 &= 0xFF00;
        IOCON->mux_ctl_5 = temp1;    
    }

    return status;
}

/***********************************************************************
 *
 * Function: ssp_ioctl
 *
 * Purpose: SSP configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate SSP
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to SSP config structure
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
STATUS ssp_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    SSP_REGS_T *ssp;
    UNS_32 tmp;
    SSP_CFG_T *sspcfgptr = (SSP_CFG_T *) devid;
    STATUS status = _ERROR;

    UNS_32 phys_source;
    UNS_32 phys_dest;
    UNS_16 * destination;
    UNS_16 * source;
    
   
    if (sspcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        ssp = sspcfgptr->regptr;

        switch (cmd)
        {
            case SSP_ENABLE:
                /* Enable SSP operation and init the SSP controller.
                arg = 1, enable SSP. arg = 0 disable SSP */
                if (arg == 1)
                {
                    /* enable SSP operation */
                    SSP->ctrl1 |= SSP_CR1_SSE;
                }
                else if (arg == 0)
                {
                    /* disable SSP operation */
                    SSP->ctrl1 &= ~SSP_CR1_SSE;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            case SSP_SET_SLAVE:
                /* Enable SSP operation in slave mode
                arg = 1, slave mode. arg = 0 master mode */
                if (arg == 1)
                {
                    /* SSP in slave mode */
                    SSP->ctrl1 |= SSP_CR1_MS;
                }
                else if (arg == 0)
                {
                    /* SSP in master mode */
                    SSP->ctrl1 &= ~SSP_CR1_MS;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                                
            case SSP_SET_FRAME_FORMAT:
                /* Set SSP in either National or TI or Motorola mode. arg =
                *   SSP_MODE_NATIONAL   : National microwire frame format
                *   SSP_MODE_TI         : TI synchronous serial data frame format
                *   SSP_MODE_MOTOROLA   : Motorola SPI data frame format
                */
                if (arg == SSP_MODE_NATIONAL)
                {
                    SSP->ctrl0 &= ~(_BIT(4) | _BIT(5));   // clear the bits
                    SSP->ctrl0 |= SSP_CR0_FRF_NS;
                }
                else if (arg == SSP_MODE_TI)
                {
                    SSP->ctrl0 &= ~(_BIT(4) | _BIT(5));   // clear the bits
                    SSP->ctrl0 |= SSP_CR0_FRF_TI;
                }
                else if (arg == SSP_MODE_MOTOROLA)
                {
                    SSP->ctrl0 &= ~(_BIT(4) | _BIT(5));   // clear the bits
                    SSP->ctrl0 |= SSP_CR0_FRF_MOT;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            case SSP_SET_DATA_SIZE:
                /* Set SSP frame data size, size is valid from 4 to 16
                arg = data size */
                if ((arg > 16) || (arg < 4))
                {
                    status = _ERROR;
                }
                else
                {
                    SSP->ctrl0 &= ~(_BIT(0) | _BIT(1) | _BIT(2)); // clear the bits
                    SSP->ctrl0 |= (arg-1);
                }
                break;
                
            case SSP_SET_SPEED:
                ssp_set_speed(arg);
                break;
                
            case SSP_SET_SSPFRM_PIN:
                /*  configure the SSPFRM pin to be high, low, 
                or auto during ssp operation. arg =
                *   SSPFRM_AUTO : set up the pin muxing so that the SSP 
                *               controls the SSPFRM pin
                *   SSPFRM_HIGH : set SSPFRM pin as GPIO and set it as high
                *   SSPFRM_LOW  : set SSPFRM pin as GPIO and set it as low
                */
                if(arg == SSPFRM_AUTO)
                {
                	IOCON->mux_ctl_5 &= ~(_BIT(0) | _BIT(1));
                	IOCON->mux_ctl_5 |= IOCON_MUX5_SSPFRM_I2SWS;
                }
                else if (arg == SSPFRM_HIGH)
                {
                    GPIOB->dr |= SSPFRM_GPIO_BIT;
                    GPIOB->ddr |= SSPFRM_GPIO_BIT; /* make GPIO an output */
                    IOCON->mux_ctl_5 &= ~(_BIT(0) | _BIT(1)); /* make SSPFRM pin a GPIO */
                }
                else if (arg == SSPFRM_LOW)
                {
                    GPIOB->dr &= ~SSPFRM_GPIO_BIT;
                    GPIOB->ddr |= SSPFRM_GPIO_BIT; /* make GPIO an output */
                    IOCON->mux_ctl_5 &= ~(_BIT(0) | _BIT(1)); /* make SSPFRM pin a GPIO */
                }
                else
                {
                    status = _ERROR;
                }
                break;
            
            case SSP_SET_SCLK_POLARITY:
                /* invert the sclk signal polarity, only effective 
                in Motorola mode. arg = 1, invert polarity, arg = 0,
                normal polarity */
                if (arg == 1)
                {
                    /* Reverse polarity */
                    SSP->ctrl0 |= SSP_CR0_SPO;
                }
                else if (arg == 0)
                {
                    /* normal polarity */
                    SSP->ctrl0 &= ~SSP_CR0_SPO;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            case SSP_SET_SCLK_PHASE:
                /* invert the sclk signal phase, only effective in 
                Motorola mode. arg = 1, invert phrase, arg = 0,
                normal phrase */
                if (arg == 1)
                {
                    /* Reverse phrase */
                    SSP->ctrl0 |= SSP_CR0_SPH;
                }
                else if (arg == 0)
                {
                    /* normal phrase */
                    SSP->ctrl0 &= ~SSP_CR0_SPH;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            case SSP_TX_WORD:
                /* transmit one word using polling mode. arg = word to be
                transfered */
                /* wait untile transmit fifo is not full */
                while((SSP->sr & SSP_SR_TNF) == 0);
                SSP->dr = (UNS_16)arg;
                break;
                
            case SSP_RX_WORD:
                /* SSP receive one word using polling mode. no arg */
                /* wait until receive fifo is not empty */
                while((SSP->sr & SSP_SR_RNE) == 0);
                status = SSP->dr;
                break;
                
            case SSP_TX_RX_WORD:
                /* SSP transceive one word using polling mode. arg =
                word to be trasmitted. Returns the word received. */
                
                /* wait untile transmit fifo is not full */
                while((SSP->sr & SSP_SR_TNF) == 0);
                SSP->dr = (UNS_16)arg;
                /* wait until receive fifo is not empty */
                while((SSP->sr & SSP_SR_RNE) == 0);
                status = SSP->dr;
                break;

            case SSP_LOOP_BACK_ENABLE:
                /* Enable SSP loop back mode. arg = 1, enable. arg = 0,
                disable */
                if (arg == 1)
                {
                    /* Enable loop back mode */
                    SSP->ctrl1 |= SSP_CR1_LBM;
                }
                else if (arg == 0)
                {
                    /* Disable loop back mode */
                    SSP->ctrl1 &= ~SSP_CR1_LBM;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                                
            case SSP_INT_ENABLE:
                /* Enable SSP interruption generation, arg =
                *   SSP_RECEIVE_FIFO_OVERRUN_INT    : enable receive fifo overrun int
                *	SSP_RECEIVE_TIMEOUT		; enable receive time out int
                *   SSP_TRANSMIT_FIFO_INT   : enable transmit fifo interruption
                *   SSP_RECEIVE_FIFO_INT    : enable receive fifo interruption
                */
                if (arg == SSP_RECEIVE_FIFO_OVERRUN_INT)
                {
                    SSP->imsc |= SSP_IMSC_RORIM;
                }
                else if (arg == SSP_RECEIVE_TIMEOUT_INT)
                {
                    SSP->imsc |= SSP_IMSC_RTIM;
                }
                else if (arg == SSP_TRANSMIT_FIFO_INT)
                {
                    SSP->imsc |= SSP_IMSC_TXIM;
                }
                else if (arg == SSP_RECEIVE_FIFO_INT)
                {
                    SSP->imsc |= SSP_IMSC_RXIM;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            case SSP_INT_DISABLE:
                /* Disable SSP interruption generation, arg = 
                *   SSP_RECEIVE_FIFO_OVERRUN_INT    : disable receive fifo overrun int
                * 	SSP_RECEIVE_TIMEOUT		; disable receive time out
                *   SSP_TRANSMIT_FIFO_INT   : disable transmit fifo interruption
                *   SSP_RECEIVE_FIFO_INT    : disable receive fifo interruption
                */
                if (arg == SSP_RECEIVE_FIFO_OVERRUN_INT)
                {
                    SSP->imsc &= ~SSP_IMSC_RORIM;
                }
                else if (arg == SSP_RECEIVE_TIMEOUT_INT)
                {
                    SSP->imsc &= ~SSP_IMSC_RTIM;
                }
                else if (arg == SSP_TRANSMIT_FIFO_INT)
                {
                    SSP->imsc &= ~SSP_IMSC_TXIM;
                }
                else if (arg == SSP_RECEIVE_FIFO_INT)
                {
                    SSP->imsc &= ~SSP_IMSC_RXIM;
                }
                else
                {
                    status = _ERROR;
                }
                break;
                
            case SSP_INT_CLEAR:
                /* only ssp receive fifo overrun interrupt can be cleared,
                no arg */
                /* clear SSP receive fifo overrun interrupt */
		        SSP->icr |= SSP_ICR_RORIC | SSP_ICR_RTIC;
                break;
                
            case SSP_DMA_TX_INIT:
                /* Set up initial value for SSP transmit using DMA, no arg */
                /* make sure DMA clock is on */
                RCPC->ahbclkctrl &= ~RCPC_AHBCLKCTRL_DMA_DISABLE;
                DMAC->stream1.ctrl = DMAC_CTRL_SOINC | DMAC_CTRL_SOSIZE_2BYTE | 
                        DMAC_CTRL_DESIZE_2BYTE | DMAC_CTRL_ADDR_MODE_WRAP |
                        DMAC_CTRL_PERIPH_DEST | DMAC_CTRL_SOBURST_4INC;
                /* clear any previous SSP DMA complete interrupts */
                DMAC->clear = DMAC_EOT0 | DMAC_EOT1;
                
				/* Enable transmit interrupt */
                DMAC->mask |= DMAC_EOT1;

                /* must generate transmit interrupt request signals to 
                request DMA */
                //ssp_set_enable(SSP_TRANSMIT_FIFO_INT);
                SSP->imsc |= SSP_IMSC_TXIM;
                
                /* disable the receiver interrupts */
                //ssp_set_disable(SSP_RECEIVE_FIFO_OVERRUN_INT);
                SSP->imsc &= ~SSP_IMSC_RORIM;
                //ssp_set_disable(SSP_RECEIVE_FIFO_INT);
                SSP->imsc &= ~SSP_IMSC_RTIM;
                SSP->imsc &= ~SSP_IMSC_RXIM;

                /* don't let the receiver or transmitter signal 
                an interrupt to the VIC */
                VIC->intenclear |= _SBF(VIC_SSPI2S, 1);

                /* Only allow DMA interrupt */                              
                /* Enable int on end of transfer */
                DMAC->mask = DMAC_EOT1;
                
                /* Enable SSP TX working in DMA mode */
                SSP->dmacr |= SSP_DMACR_TXDMAE;
                
                break;
                
            case SSP_DMA_RX_INIT:
                /* Set up initial value for SSP receive using DMA, no arg */
                /* make sure DMA clock is on */
                RCPC->ahbclkctrl &= ~RCPC_AHBCLKCTRL_DMA_DISABLE;
                DMAC->stream1.ctrl = DMAC_CTRL_SOSIZE_2BYTE | 
                        DMAC_CTRL_DESIZE_2BYTE | DMAC_CTRL_ADDR_MODE_WRAP |
                        DMAC_CTRL_PERIPH_DEST | DMAC_CTRL_SOBURST_4INC;
                DMAC->stream0.ctrl = DMAC_CTRL_DEINC | DMAC_CTRL_SOSIZE_2BYTE | 
                        DMAC_CTRL_DESIZE_2BYTE | DMAC_CTRL_ADDR_MODE_WRAP |
                        DMAC_CTRL_PERIPH_SOURCE | DMAC_CTRL_SOBURST_4INC;
                /* 
                Chip issue work-around (see errata sheet)
                enable receive DMA amd then disable it to prevent 
                bad initial DMA transfer from receive FIFO.
                */
                DMAC->stream0.ctrl |= DMAC_CTRL_ENABLE;
                DMAC->stream0.ctrl = DMAC_CTRL_DEINC | DMAC_CTRL_SOSIZE_2BYTE | 
                        DMAC_CTRL_DESIZE_2BYTE | DMAC_CTRL_ADDR_MODE_WRAP |
                        DMAC_CTRL_PERIPH_SOURCE | DMAC_CTRL_SOBURST_4INC;
                        
   
                /* clear any previous SSP DMA complete interrupts */
                DMAC->clear = DMAC_EOT0 | DMAC_EOT1;
                
				/* Enable receive interrupt */
                DMAC->mask |= DMAC_EOT0 ;

                /* must generate interrupt request signals to request DMA */
                //ssp_set_enable(SSP_TRANSMIT_FIFO_INT);
                SSP->imsc |= SSP_IMSC_TXIM;
                //ssp_set_enable(SSP_RECEIVE_FIFO_INT);
                SSP->imsc |= SSP_IMSC_RXIM;
                
                /* disable the receiver overflow interrupt 
                (it shouldn't happen) */
                //ssp_set_disable(SSP_RECEIVE_FIFO_OVERRUN_INT);
                SSP->imsc &= ~SSP_IMSC_RTIM;
                SSP->imsc &= ~SSP_IMSC_RORIM;
                
                /* set up the SSP DMA receiver timeout */
                //bps = ssp_get_speed();
                //if (bps == 0)
                //{
                    //status = _ERROR;
                    //break; /* ssp was not initialized properly */
                //}
                //bpf = ssp_get_speed();
                //if ( (SSP->ctrl0 & SSP_CR0_FRF_NS) == SSP_CR0_FRF_NS)
                    //bpf += 8; /* the control word is sent first */
   
                /* 
                timeout in microseconds for a frame at the watermark 
                is (5 frames in FIFO * 1000000 us/s * bpf bits/frame)
                 / (bps bits/s) */
                //ssp_set_dma_receive_timeout( (5 * 1000000 * bpf) / bps );   
   
                /* don't let the receiver or transmitter signal an 
                interrupt to the VIC */
                //int_command(VIC_SSPRX, SOURCE_INT_DISABLE);
                //int_command(VIC_SSPTX, SOURCE_INT_DISABLE);
                //int_command(VIC_SSPROR,SOURCE_INT_DISABLE);
                //int_command(VIC_SSPINT,SOURCE_INT_DISABLE);
                VIC->intenclear |= _SBF(VIC_SSPI2S, 1);

                /* don't let the DMA controller signal an interrupt to the VIC */
                //int_command(VIC_DMA,SOURCE_INT_DISABLE);
                VIC->intenclear |= _SBF(VIC_DMA0, 1);
                
                /* Enable SSP RX working in DMA mode */
                SSP->dmacr |= SSP_DMACR_RXDMAE;
                
                break;
                
            case SSP_DMA_TX_RX_INIT:
                /* Set up initial value for SSP transceive using DMA, no arg */
                /* make sure DMA clock is on */
                RCPC->ahbclkctrl &= ~RCPC_AHBCLKCTRL_DMA_DISABLE;
                DMAC->stream1.ctrl = DMAC_CTRL_SOINC | DMAC_CTRL_SOSIZE_2BYTE | 
                        DMAC_CTRL_DESIZE_2BYTE | DMAC_CTRL_ADDR_MODE_WRAP |
                        DMAC_CTRL_PERIPH_DEST | DMAC_CTRL_SOBURST_4INC;
                DMAC->stream0.ctrl = DMAC_CTRL_DEINC | DMAC_CTRL_SOSIZE_2BYTE | 
                        DMAC_CTRL_DESIZE_2BYTE | DMAC_CTRL_ADDR_MODE_WRAP |
                        DMAC_CTRL_PERIPH_SOURCE | DMAC_CTRL_SOBURST_4INC;
                        
                /* clear any previous SSP DMA complete interrupts */
                DMAC->clear = DMAC_EOT0 | DMAC_EOT1;

				/* Enable receive and transmit interrupt */
                DMAC->mask |= DMAC_EOT0 | DMAC_EOT1;

                /* must generate interrupt request signals to request DMA */
                //ssp_set_enable(SSP_TRANSMIT_FIFO_INT);
                SSP->imsc |= SSP_IMSC_TXIM;
                //ssp_set_enable(SSP_RECEIVE_FIFO_INT);
                SSP->imsc |= SSP_IMSC_RXIM;
                
                /* disable the receiver overflow interrupt 
                (it shouldn't happen) */
                //ssp_set_disable(SSP_RECEIVE_FIFO_OVERRUN_INT);
                SSP->imsc &= ~SSP_IMSC_RORIM;
                SSP->imsc &= ~SSP_IMSC_RTIM;
                
                /* set up the SSP DMA receiver timeout */
                //bps = ssp_get_speed();
                //if (bps == 0)
                //{
                    //status = _ERROR;
                    //break; /* ssp was not initialized properly */
                //}
                //bpf = ssp_get_speed();
                //if ( (SSP->ctrl0 & SSP_CR0_FRF_NS) == SSP_CR0_FRF_NS)
                    //bpf += 8; /* the control word is sent first */
   
                /* 
                timeout in microseconds for a frame at the watermark 
                is (5 frames in FIFO * 1000000 us/s * bpf bits/frame)
                 / (bps bits/s)   */
                //ssp_set_dma_receive_timeout( (5 * 1000000 * bpf) / bps );   
   
                /* don't let the receiver or transmitter signal an 
                interrupt to the VIC */
                //int_command(VIC_SSPRX, SOURCE_INT_DISABLE);
                //int_command(VIC_SSPTX, SOURCE_INT_DISABLE);
                //int_command(VIC_SSPROR,SOURCE_INT_DISABLE);
                //int_command(VIC_SSPINT,SOURCE_INT_DISABLE);
                VIC->intenclear |= _SBF(VIC_SSPI2S, 1);
                
                /* don't let the DMA controller signal an interrupt to the VIC */
                //int_command(VIC_DMA,SOURCE_INT_DISABLE);
                VIC->intenclear |= _SBF(VIC_DMA1, 1);
                VIC->intenclear |= _SBF(VIC_DMA0, 1);
                
                /* Enable SSP RX and TX working in DMA mode */
                SSP->dmacr |= SSP_DMACR_TXDMAE;
                SSP->dmacr |= SSP_DMACR_RXDMAE;
                                
                break;
                
            case SSP_DMA_SET_SOURCE:
                /* Set up source address for SSP using DMA, arg = source addr */
                /* make sure DMA clock is on */
                RCPC->ahbclkctrl &= ~RCPC_AHBCLKCTRL_DMA_DISABLE;
//                phys_source = mmu_map_virtual_to_physical(arg);
                phys_source = (arg);
                
                DMAC->stream1.sourcelo = (phys_source) & 0xffff;
                DMAC->stream1.sourcehi = (phys_source) >> 16;
                DMAC->stream1.destlo = ((UNS_32)&(SSP->dr)) & 0xffff;
                DMAC->stream1.desthi = ((UNS_32)&(SSP->dr)) >> 16;
                break;
                
            case SSP_DMA_SET_DEST:
                /* Set up destination address for SSP using DMA, 
                arg = destination addr */
                /* make sure DMA clock is on */
                RCPC->ahbclkctrl &= ~RCPC_AHBCLKCTRL_DMA_DISABLE;
//                phys_dest = mmu_map_virtual_to_physical(arg);
                phys_dest = (arg);
                DMAC->stream0.sourcelo = ((UNS_32)&(SSP->dr)) & 0xffff;
                DMAC->stream0.sourcehi = ((UNS_32)&(SSP->dr)) >> 16;
                DMAC->stream0.destlo = (phys_dest) & 0xffff;
                DMAC->stream0.desthi = (phys_dest) >> 16;
                break;
                
            case SSP_DMA_SET_COUNT:
                /* Set up number of words for SSP operation using DMA,
                arg = number of words */
                /* make sure DMA clock is on */
                RCPC->ahbclkctrl &= ~RCPC_AHBCLKCTRL_DMA_DISABLE;
                DMAC->stream1.max = arg;
                DMAC->stream0.max = arg;
                break;
                
            case SSP_DMA_TX_INT_HANDLER:
                /* Default transmit interrupt handler to be called by
                SSP DMA transfer interrupt routine, no arg */
                /* disable transmit DMA */ 
                DMAC->stream1.ctrl &= ~DMAC_CTRL_ENABLE;
                /* clear any DMA interrupts signaled by the SSP */
                DMAC->clear = DMAC_EOT0 | DMAC_EOT1;
                break;              
                
            case SSP_DMA_TXRX_INT_HANDLER:
                /* Default receive and transceive interrupt handler 
                to be called by SSP DMA transfer interrupt routine, 
                no arg */
                
                /* Wait until receive time out */
                //while(((SSP->ris & SSP_RIS_RTRIS)) == 0);

                /* disable receive DMA */
                DMAC->stream0.ctrl &= ~DMAC_CTRL_ENABLE;
                /* disable transmit DMA */ 
                DMAC->stream1.ctrl &= ~DMAC_CTRL_ENABLE;
                /* clear any DMA interrupts signaled by the SSP */
                DMAC->clear = DMAC_EOT0 | DMAC_EOT1;
                phys_dest = (DMAC->stream0.decurrlo | 
                            (DMAC->stream0.decurrhi << 16) );
//                destination = (UNS_16 *)mmu_map_physical_to_virtual(phys_dest);
                destination = (UNS_16 *)(phys_dest);
                
                for (tmp = 0; tmp < DMAC->stream0.tcnt; tmp++)
                {
                    /* receive the the remaining words */
                    *destination++ = SSP->dr;
                }
                /* clear the receiver timeout interrupt 
                and prevent further timeouts */
                //SSP->rxto = 0;
                break;              
                
            case SSP_DMA_TX_START:
                /* Start DMA transfer from memory to SSP, SSP_DMA_TX_INIT
                must be called before this, no arg */
                DMAC->stream1.ctrl |= DMAC_CTRL_ENABLE;
                break;
                
            case SSP_DMA_RX_START:
                /* Start DMA transfer from SSP to memory, SSP_DMA_RX_INIT
                must be called before this, no arg */
                /* If there are not enough words to transfer to trigger at least
                *  one SSP receive interrupt, then do the transfer using non-DMA
                *  operations.
                *  Otherwise, set the stream0 control register enable bit to arm
                *  SSP receive DMA.
                *  Set the stream1 control register enable bit to start the DMA
                *  transfer out of the SSP. */
                if (DMAC->stream1.max <= 5)
                {
                    /* DMA won't time out. do the transfer manually */
                    UNS_16 i, * destination;
      
//                    destination = (UNS_16 *)mmu_map_physical_to_virtual
//                        (DMAC->stream0.destlo | (DMAC->stream0.desthi << 16 ));
                    destination = (UNS_16 *)(DMAC->stream0.destlo | (DMAC->stream0.desthi << 16 ));
                        
                    for (i = 0; i< DMAC->stream1.max; i++)
                    {
                        SSP->dr = 0;
                        destination[i] = SSP->dr;
                    }
                }
                else
                {
                    /* enable receive DMA */
                    DMAC->stream0.ctrl |= DMAC_CTRL_ENABLE;
                    /* enable transmit DMA (send zero; transfer starts 
                    immediately) */ 
                    DMAC->stream1.ctrl |= DMAC_CTRL_ENABLE;
                }
                break;
                
            case SSP_DMA_TX_RX_START:
                /*  Start DMA transfer from SSP to memory, SSP_DMA_TX_RX_INIT
                must be called before this, no arg */
                /* If there are not enough words to transfer to trigger at least
                *  one SSP receive interrupt, then do the transfer using non-DMA
                *  operations.
                *  Otherwise, set the stream0 control register enable bit to arm
                *  SSP receive DMA.
                *  Set the stream1 control register enable bit to start the DMA
                *  transfer out of the SSP. */
                tmp = DMAC->stream1.max;
                if (DMAC->stream1.max <= 5)
                {
                    /* DMA won't time out. do the transfer manually */
//                    source = (UNS_16 *)mmu_map_physical_to_virtual
//                        (DMAC->stream1.sourcelo | (DMAC->stream1.sourcehi << 16 ));
                    source = (UNS_16 *)(DMAC->stream1.sourcelo | (DMAC->stream1.sourcehi << 16 ));
//                    destination = (UNS_16 *)mmu_map_physical_to_virtual
//                        (DMAC->stream0.destlo | (DMAC->stream0.desthi << 16 ));
                    destination = (UNS_16 *)(DMAC->stream0.destlo | (DMAC->stream0.desthi << 16 ));
                    
                    while(tmp--)
                    {
                        SSP->dr = *source++;
                        *destination++ = SSP->dr;
                    }
                }
                else
                {
                    /* enable receive DMA */
                    DMAC->stream0.ctrl |= DMAC_CTRL_ENABLE;
                    /* enable transmit DMA (transfer starts immediately) */ 
                    DMAC->stream1.ctrl |= DMAC_CTRL_ENABLE;
                }
                break;
                
            case SSP_GET_STATUS:
                switch (arg)
                {
                    case SSP_GET_SPEED:
                        status = ssp_get_speed();
                        break;
                        
                    case SSP_GET_INT_STATUS:
                        /* Returns SSP interrupt status */
                        tmp = SSP->mis;
                        if ((tmp & SSP_MIS_RORMIS) != 0)
                        {
                            status = SSP_RECEIVE_FIFO_OVERRUN_INT;
                        }
                        else if ((tmp & SSP_MIS_TXMIS) != 0)
                        {
                            status = SSP_TRANSMIT_FIFO_INT;
                        }
                        else if ((tmp & SSP_MIS_RXMIS) != 0)
                        {
                            status = SSP_RECEIVE_FIFO_INT;
                        }
                        else if ((tmp & SSP_ICR_RTIC) != 0)
                        {
                            status = SSP_RECEIVE_TIMEOUT_INT;
                        }
                        else
                        {
                            status = _ERROR;
                        }
                        break;

                    case SSP_GET_BUSY:
                        /* Returns 1 if SSP is busy, 0 for not busy */
                        if ((SSP->sr & SSP_SR_BSY) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;
                        
                    case SSP_GET_RX_FIFO_FULL:
                        /* Returns 1 if SSP receive fifo is full, 
                        0 for not full */
                        if ((SSP->sr & SSP_SR_RFF) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;
                        
                    case SSP_GET_RX_FIFO_NOT_EMPTY:
                        /* Returns 1 if SSP receive fifo is not empty, 
                        0 for empty */
                        if ((SSP->sr & SSP_SR_RNE) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;
                        
                    case SSP_GET_TX_FIFO_NOT_FULL:
                        /* Returns 1 if SSP transmit fifo is not full, 
                        0 for full */
                        if ((SSP->sr & SSP_SR_TNF) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;
                        
                    case SSP_GET_TX_FIFO_EMPTY:
                        /* Returns 1 if SSP transmit fifo is empty, 
                        0 for not empty */
                        if ((SSP->sr & SSP_SR_TFE) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
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
 * Function: ssp_read
 *
 * Purpose: SSP read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to SSP config structure
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
INT_32 ssp_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: ssp_write
 *
 * Purpose: SSP write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to SSP config structure
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
INT_32 ssp_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}
