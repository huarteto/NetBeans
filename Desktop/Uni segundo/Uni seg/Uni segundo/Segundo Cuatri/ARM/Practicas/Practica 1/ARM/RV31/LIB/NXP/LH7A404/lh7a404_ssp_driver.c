/***********************************************************************
 * $Id:: lh7a404_ssp_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 SSP driver
 *
 * Description:
 *     This file contains driver support for the SSP module on the
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
#include "lh7a404_ssp_driver.h"

/***********************************************************************
 * SSP driver private data and types
 **********************************************************************/

/* Speed of SSP peripheral clock */
#define SSP_CLOCK (CLOCK_MAINOSC / 2)

/* Size of SSP transmit and receive queues (ring buffers) */
#ifndef SSP_R_SIZE
#define SSP_R_SIZE 64
#endif

/* SSP device configuration structure type */
typedef struct
{
    BOOL_32 init;          /* Device initialized flag */
    UNS_16 tx[SSP_R_SIZE]; /* SSP TX data */
    UNS_16 rx[SSP_R_SIZE]; /* SSP RX data */
    INT_32 tx_head;        /* SSP TX 'put' entry index */
    INT_32 tx_tail;        /* SSP TX 'get' entry index */
    INT_32 rx_head;        /* SSP RX 'put' entry index */
    INT_32 rx_tail;        /* SSP RX 'get' entry index */
    SSP_REGS_T *regptr;    /* Pointer to SSP registers */
} SSP_CFG_T;

/* SSP device configuration structure */
STATIC SSP_CFG_T sspcfg;

/* SSP default clock rate when initialized */
#define DEFAULT_SSP_CLOCK (CLOCK_MAINOSC / 16)

/***********************************************************************
 * SSP driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: ssp_set_clock
 *
 * Purpose: Sets or resets the serial clock rate of the SSP interface
 *          (in Hz)
 *
 * Processing:
 *     Set the default divider and prescaler divider values to 1. Read
 *     the existing control 0 word and mask off the clock rate bits.
 *     Compute the optimum divider valye with a call to
 *     csc_compute_divider with the target_clock frequency and the
 *     default SSP peripheral clock. If the divider is 1, update the
 *     SSP control 0 register and SSP prescaler register with values
 *     to generate the fastest SSP clock available and exit. If the
 *     computed main divider is over 254, then limit the computed
 *     main divider to 254 and compute the prescaler divider by
 *     increment the prescaler divider and testing it against the
 *     computed frequency with the main SSP divider. Before exiting,
 *     write the computed main clock divider and prescaler divider
 *     values to the SSP control 0 and prescaler registers.
 *
 * Parameters:
 *     ssp_ptr      : Pointer to SSP registers
 *     target_clock : The value in Hz for the new SSP serial clock
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void ssp_set_clock(SSP_REGS_T *ssp_ptr, UNS_32 target_clock)
{
    UNS_32 control, prescale, ssp_div;

    /* Always a minimum divider and prescaler value of 1 */
    ssp_div = 1;
    prescale = 1;

    /* Get existing control word with clock data and clear prescale */
    control = ssp_ptr->cr0 &= ~(SSP_CR0_SCR(255));

    /* The SSP clock is derived from the (main system oscillator / 2),
       so compute the best divider from that clock */
    ssp_div = csc_compute_divider(SSP_CLOCK, target_clock);

    /* If divider is above 254, then the prescaler is needed also */
    if (ssp_div >= 255)
    {
        /* Limit SSP main divider to 254 (which corresponds to a 255
           divider) */
        ssp_div = 254;

        /* Find the closest target clock frequency with the prescaler */
        while ((prescale < 254) &&
            (target_clock < (SSP_CLOCK / (prescale + ssp_div + 1))))
        {
            /* Try next prescale value */
            prescale++;
        }
    }

    /* Write computed prescaler and divider back to register */
    ssp_ptr->cr0 = control | SSP_CR0_SCR(ssp_div - 1);
    ssp_ptr->cpsr = prescale;
}

/***********************************************************************
 *
 * Function: ssp_get_free_tx_count
 *
 * Purpose: Returns the number of free spaces left in the TX queue
 *
 * Processing:
 *     Compute count by subtracting the tx_tail value from the tx_head
 *     value for the transmit queue. If the value is less than 0, then
 *     compute count by subtracting tx_tail from the queue size and
 *     adding in the tx_head index. Computed the 'used' value by
 *     subtracting the (count value + 1) from the queue size and return
 *     this value to the caller.
 *
 * Parameters:
 *     sspcfgptr : Pointer to an SSP config structure
 *
 * Outputs: None
 *
 * Returns: The number of free data entries in the TX ring buffer
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_get_free_tx_count(SSP_CFG_T *sspcfgptr)
{
    INT_32 count;

    count = sspcfgptr->tx_head - sspcfgptr->tx_tail;
    if (count < 0)
    {
        /* Head pointer has flipped to start of ring */
        count = (SSP_R_SIZE - sspcfgptr->tx_tail) +
            sspcfgptr->tx_head;
    }

   return (SSP_R_SIZE - count - 1);
}

/***********************************************************************
 *
 * Function: ssp_standard_receive
 *
 * Purpose: SSP data receive function
 *
 * Processing:
 *     While the receive FIFO is not empty, read the data from the FIFO
 *     and place it into the receive queue indexed by the rx_head index.
 *     Increment the rx_head index. If the rx_head_index goes past the
 *     size of the queue end index (SSP_R_SIZE), reset the rx_head_index
 *     to 0. Continue reading until the FIFO is empty.
 *
 * Parameters:
 *     sspcfgptr : Pointer to an SSP config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void ssp_standard_receive(SSP_CFG_T *sspcfgptr)
{
    SSP_REGS_T *sspregs = sspcfgptr->regptr;

    /* Is there data in the RX FIFO? */
    while ((sspregs->sr & SSP_SR_RNE) != 0)
    {
        /* Read data from SSP and put in queue */
        sspcfgptr->rx[sspcfgptr->rx_head] = (UNS_16) sspregs->dr;

        /* Increment and limit rx queue head pointer */
        sspcfgptr->rx_head++;

        /* Reindex queue to start if needed */
        if (sspcfgptr->rx_head >= SSP_R_SIZE)
        {
            sspcfgptr->rx_head = 0;
        }
    }
}

/***********************************************************************
 *
 * Function: ssp_standard_transmit
 *
 * Purpose: SSP data transmit function
 *
 * Processing:
 *     While the transmit queue is not empty and the SSP transmit FIFO
 *     has space in it, place the data value from the transmit queue
 *     indexed by tx_tail into the SSP transmit FIFO. Increment tx_tail.
 *     If tx_tail goes past the size of the queue end index
 *     (SSP_R_SIZE), reset the rx_head_index to 0. If the transmit FIFO
 *     is empty, disable the transmit FIFO service interrupt, otherwise
 *     enable it.
 *
 * Parameters:
 *     sspcfgptr : Pointer to an SSP config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void ssp_standard_transmit(SSP_CFG_T *sspcfgptr)
{
    SSP_REGS_T *sspregs = sspcfgptr->regptr;

    /* Continue until all data is sent or until TX FIFO is full */
    while ((sspcfgptr->tx_head != sspcfgptr->tx_tail) &&
        ((sspregs->sr & SSP_SR_TNF) != 0))
    {
        /* Put entry in FIFO */
        sspregs->dr = (UNS_32) sspcfgptr->tx[sspcfgptr->tx_tail];
            
        /* Index to next transmit FIFO entry */
        sspcfgptr->tx_tail++;

        /* Re-index queue to start if needed */
        if (sspcfgptr->tx_tail >= SSP_R_SIZE)
        {
            sspcfgptr->tx_tail = 0;
        }
    }

    /* If the transmit queue is empty, disable the transmit interrupt,
       otherwise enable it */
    if (sspcfgptr->tx_head == sspcfgptr->tx_tail)
    {
        /* Empty queue, no more transmit interrupts */
        sspregs->cr1 &= ~SSP_CR1_TIE;
    }
    else
    {
        /* Enable or keep transmit interrupts enabled */
        sspregs->cr1 |= SSP_CR1_TIE;
    }
}

/***********************************************************************
 *
 * Function: ssp_standard_interrupt
 *
 * Purpose: SSP standard interrupt function
 *
 * Processing:
 *     If the receive FIFO interrupt is pending, call the
 *     ssp_standard_receive function. If the transmit FIFO empty
 *     interrupt is pending, call the ssp_standard_transmit function.
 *     If the receive FIFO overrun interrupt is pending, disable and
 *     clear the receive overrun interrupt. If the transmitter idle
 *     interrupt is pending, disable the transmitter idle interrupt.
 *
 * Parameters:
 *     sspcfgptr : Pointer to an SSP config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void ssp_standard_interrupt(SSP_CFG_T *sspcfgptr)
{
    SSP_REGS_T *sspregs = sspcfgptr->regptr;

    /* Interrupt was due to a receive data FIFO service request */
    if ((sspregs->iir_icr & SSP_IIR_RIS) != 0)
    {
        /* Receive interrupt */
        ssp_standard_receive(sspcfgptr);
    }

    /* Interrupt was due to a transmit data FIFO service request */
    if ((sspregs->iir_icr & SSP_IIR_TIS) != 0)
    {
        /* Transmit interrupt */
        ssp_standard_transmit(sspcfgptr);
    }

    /* Interrupt was due to a receive FIFO overrun service request */
    if ((sspregs->iir_icr & SSP_IIR_RORIS) != 0)
    {
        /* RX FIFO overrun, just clear interrupt and disable it */
        sspregs->iir_icr = 0;
        sspregs->cr1 &= ~SSP_CR1_RORIE;
    }

    /* Interrupt was due to a transmitter idle service request */
    if ((sspregs->iir_icr & SSP_IIR_TXIDLE) != 0)
    {
        /* Disable this interrupt, not used */
        sspregs->cr1 &= ~SSP_CR1_TXIDLE;
    }
}

/***********************************************************************
 * SSP driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: ssp_open
 *
 * Purpose: Open the SSP
 *
 * Processing:
 *     If init is not FALSE, return NULL to the caller. Otherwise,
 *     set init to TRUE, save the SSP peripheral register set address,
 *     set the default states of the SSP device (SSP disabled, SSP clock
 *     is divide by 8, SPI mode, FIFOs enabled), and return a pointer to
 *     the SSP config structure to the caller.
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

    if ((sspcfg.init == FALSE) && ((SSP_REGS_T *) ipbase == SSP))
    {
        /* Device is valid and not previously initialized */
        sspcfg.init = TRUE;

        /* Save and return address of peripheral block */
        sspcfg.regptr = (SSP_REGS_T *) ipbase;

        /* Disable SSP and set default clock to divide by 8,
           Motorola SPI mode, and 16 data bits */
        sspcfg.regptr->cr0 = (SSP_CR0_FRF_MOT | SSP_CR0_SCR(7) |
            SSP_CR0_DSS(16));

        /* Set default FIFO to enabled, loopback mode disabled, all
           interrupts disabled, and SPH and SPI to 0 */
        sspcfg.regptr->cr1 = SSP_CR1_FEN;

        /* Set SSP clock prescaler */
        ssp_set_clock(sspcfg.regptr, DEFAULT_SSP_CLOCK);

        /* Return pointer to RTC configuration structure */
        status = (INT_32) &sspcfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: ssp_close
 *
 * Purpose: Close the SSP
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, set init to FALSE,
 *     disable the SSP device, and return _NO_ERROR to the caller.
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

    if (sspcfgptr->init == TRUE)
    {
        /* 'Uninitialize' device */
        sspcfgptr->init = FALSE;
        status = _NO_ERROR;

        /* Disable device */
        sspcfg.regptr->cr0 = 0x00000000;
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
 *     and option values, set or get the appropriate SSP parameter.
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
    SSP_REGS_T *sspregs;
    UNS_32 tmp, tmp2;
    SSP_CFG_T *sspcfgptr = (SSP_CFG_T *) devid;
    STATUS status = _ERROR;

    if (sspcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        sspregs = sspcfgptr->regptr;

        switch (cmd)
        {
            case SSP_ENABLE:
                if (arg == 1)
                {
                    /* Enable SSP */
                    sspregs->cr0 |= SSP_CR0_SSE;
                }
                else
                {
                    /* Disable SSP */
                    sspregs->cr0 &= ~SSP_CR0_SSE;
                }
                break;

            case SSP_SET_PROTOCOL:
                /* Change SSP protocol state */
                tmp = sspregs->cr0 & ~SSP_CR0_PRT_MSK;
                switch (arg)
                {
                    case SSP_MOTOROLA_SPI:
                        /* SPI mode */
                        sspregs->cr0 = (tmp | SSP_CR0_FRF_MOT);
                        break;

                    case SSP_TI_SSI:
                        /* SSI mode */
                        sspregs->cr0 = (tmp | SSP_CR0_FRF_TI);
                        break;

                    case SSP_MICROWIRE:
                        /* Microwire mode */
                        sspregs->cr0 = (tmp | SSP_CR0_FRF_NS);
                        break;

                    default:
                        /* Unsupported parameter */
                        status = SMA_BAD_PARAMS;
                        break;
                }
                break;

            case SSP_SET_CLOCK:
                /* Change SSP clock */
                ssp_set_clock(sspregs, (UNS_32) arg);
                break;

            case SSP_SET_DATA_BITS:
                /* Change number of data bits */
                if ((arg >= 4) && (arg <= 16))
                {
                    /* Good data bit size */
                    tmp = sspregs->cr0 & ~SSP_CR0_DSS(16);
                    sspregs->cr0 = (tmp | SSP_CR0_DSS(arg));
                }
                else
                {
                    /* Invalid data bit size */
                    status = SMA_BAD_PARAMS;
                }
                break;

            case SSP_ENABLE_FIFO:
                /* Enable or disable SSP FIFO */
                if (arg == 1)
                {
                    /* Enable SSP FIFO */
                    sspregs->cr1 |= SSP_CR1_FEN;
                }
                else
                {
                    /* Disable SSP FIFO */
                    sspregs->cr1 &= ~SSP_CR1_FEN;
                }
                break;

            case SSP_ENABLE_LOOPB:
                /* Enable or disable loopback mode */
                if (arg == 1)
                {
                    /* Enable SSP loopback mode */
                    sspregs->cr1 |= SSP_CR1_LBM;
                }
                else
                {
                    /* Disable SSP loopback mode */
                    sspregs->cr1 &= ~SSP_CR1_LBM;
                }
                break;

            case SSP_ENABLE_INTS:
                /* Enable SSP interrupts */
                /* Mask off all bits except interrupt bits */
                tmp = (UNS_32) arg & (SSP_CR1_RIE | SSP_CR1_TIE |
                    SSP_CR1_RORIE | SSP_CR1_TXIDLE);

                /* Get old control word and mask off old interrupt
                   bits */
                tmp2 = sspregs->cr1 & ~(SSP_CR1_RIE | SSP_CR1_TIE |
                    SSP_CR1_RORIE | SSP_CR1_TXIDLE);

                /* OR in new interrupt bits */
                sspregs->cr1 = (tmp | tmp2);
                break;

            case SSP_DISABLE_INTS:
                /* Disable SSP interrupts */
                /* Mask off all bits except interrupt bits */
                tmp = (UNS_32) arg & (SSP_CR1_RIE | SSP_CR1_TIE |
                    SSP_CR1_RORIE | SSP_CR1_TXIDLE);

                /* OR in new interrupt bits */
                sspregs->cr1 &= ~tmp;
                break;

            case SSP_CLEAR_OVR_INT:
                /* Clear receive overrun bit */
                sspregs->iir_icr = 0x00000000;
                break;

              case SSP_SET_SPI_PHASE:
                  /* Set the state of the SPH bit */
                if (arg == 1)
                {
                    /* Set SPH to 1 */
                    sspregs->cr1 |= SSP_CR1_SPH;
                }
                else
                {
                    /* Set SPH to 0 */
                    sspregs->cr1 &= ~SSP_CR1_SPH;
                }
                break;

              case SSP_SET_SPI_POL:
                  /* Set the state of the SPO bit */
                if (arg == 1)
                {
                    /* Set SPO to 1 */
                    sspregs->cr1 |= SSP_CR1_SPO;
                }
                else
                {
                    /* Set SPO to 0 */
                    sspregs->cr1 &= ~SSP_CR1_SPO;
                }
                break;

            case SSP_GET_STATUS:
                /* Return an SSP status */
                switch (arg)
                {
                    case SSP_ENABLED_ST:
                        if ((sspregs->cr0 & SSP_CR0_SSE) != 0)
                        {
                            /* SSP is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* SSP is disabled */
                            status = 0;
                        }
                        break;

                    case SSP_PROTOCOL_ST:
                        /* Return SSP protocol state */
                        switch (sspregs->cr0 & SSP_CR0_PRT_MSK)
                        {
                            case SSP_CR0_FRF_MOT:
                                /* SPI mode */
                                status = SSP_MOTOROLA_SPI;
                                break;

                            case SSP_CR0_FRF_TI:
                                /* SSI mode */
                                status = SSP_TI_SSI;
                                break;

                            case SSP_CR0_FRF_NS:
                                /* Microwire mode */
                                status = SSP_MICROWIRE;
                                break;

                            default:
                                /* Unknown mode, this should never
                                   happen */
                                status = _ERROR;
                                break;
                        }
                        break;

                    case SSP_CLOCK_ST:
                        /* Return clock speed of SSP interface */
                        tmp = (sspregs->cr0 & SSP_CR0_SCR(0xFF)) >> 8;
                        tmp2 = sspregs->cpsr;
                        if (tmp2 < 1)
                        {
                            /* Not a valid value, so use a divider of 1 */
                            tmp2 = 1;
                        }

                        /* Compute SSP bit clock rate */
                        status = SSP_CLOCK / (tmp2 + tmp + 1);
                        break;

                    case SSP_DATA_BITS:
                        /* Returns number of data bits */
                        status = ((INT_32) sspregs->cr0 & 0xF) + 1;
                        break;

                    case SSP_FIFO_ENABLE_ST:
                        /* Return FIFO enabled status */
                        if ((sspregs->cr1 & SSP_CR1_FEN) != 0)
                        {
                            /* FIFO enabled */
                            status = 1;
                        }
                        else
                        {
                            /* FIFO disabled */
                            status = 0;
                        }
                        break;

                    case SSP_LOOPB_ST:
                        /* Return loopback mode enabled status */
                        if ((sspregs->cr1 & SSP_CR1_LBM) != 0)
                        {
                            /* Lopback mode enabled */
                            status = 1;
                        }
                        else
                        {
                            /* Lopback mode disabled */
                            status = 0;
                        }
                        break;

                    case SSP_INTS_ST:
                        /* Return enabled interrupts */
                        status = (INT_32) (sspregs->cr1 & (SSP_CR1_RIE |
                            SSP_CR1_TIE | SSP_CR1_RORIE |
                            SSP_CR1_TXIDLE));
                        break;

                    case SSP_SPI_PHASE_ST:
                        /* Returns state of SPH bit */
                        if ((sspregs->cr1 & SSP_CR1_SPH) != 0)
                        {
                            /* SPH is high */
                            status = 1;
                        }
                        else
                        {
                            /* SPH is low */
                            status = 0;
                        }
                        break;


                    case SSP_SPI_POL_ST:
                        /* Returns state of SPO bit */
                        if ((sspregs->cr1 & SSP_CR1_SPO) != 0)
                        {
                            /* SPO is high */
                            status = 1;
                        }
                        else
                        {
                            /* SPO is low */
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
 * Function: ssp_read_polled
 *
 * Purpose: SSP polled read function
 *
 * Processing:
 *     If the init flag for the SSP structure is FALSE, return 0 to
 *     the caller. Otherwise, loop until "max_fifo" equals 0 or until
 *     the receive FIFO is empty, whichever comes first. Read the data
 *     from the SSP FIFO and place it user buffer. Increment the
 *     address of the user buffer. Increment "count", and decrement
 *     "max_fifo". Exit the loop based on the loop conditions and
 *     return the number of times the fifo is read to the caller.
 *
 * Parameters:
 *     devid:     Pointer to SSP config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_fifo:  Number of times to read the receive fifo
 *
 * Outputs: None
 *
 * Returns: Number of times the receive fifo was actually read
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_read_polled(INT_32 devid,
                       void *buffer,
                       INT_32 max_fifo)
{
    SSP_REGS_T *sspregs;
    SSP_CFG_T *sspcfgptr = (SSP_CFG_T *) devid;
    UNS_16 *data = (UNS_16 *) buffer;
    INT_32 count = 0;

    if (sspcfgptr->init == TRUE)
    {
        sspregs = sspcfgptr->regptr;

        /* Loop until receive FIFO is empty or until max_fifo
           expires */
        while ((max_fifo > 0) &&
            ((sspregs->sr & SSP_SR_RNE) != 0))
        {
            /* Read data from FIFO into buffer */
            *data = (UNS_16) sspregs->dr;
            data++;

            /* Increment data count and decrement buffer size count */
            count++;
            max_fifo--;
        }
    }

    return count;
}

/***********************************************************************
 *
 * Function: ssp_write_polled
 *
 * Purpose: SSP polled write function
 *
 * Processing:
 *     If the init flag for the SSP structure is FALSE, return 0 to
 *     the caller. Otherwise, loop until "n_fifo" equals 0. Loop until
 *     the transmit FIFO is full inside the "n_fifo" loop. Read the data
 *     from the user buffer and place it in the SSP FIFO. Increment
 *     the address of the user buffer. Increment "count", and decrement
 *     "n_fifo". Exit the loop based on the loop conditions and
 *     return the number of times the fifo was written to the caller.
 *
 * Parameters:
 *     devid:   Pointer to SSP config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_fifo:  Number of times to write data to the transmit fifo
 *
 * Outputs: None
 *
 * Returns: Number of times the transmit fifo was actually written
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_write_polled(INT_32 devid,
                        void *buffer,
                        INT_32 n_fifo)
{
    SSP_REGS_T *sspregs;
    SSP_CFG_T *sspcfgptr = (SSP_CFG_T *) devid;
    UNS_16 *data = (UNS_16 *) buffer;
    INT_32 count = 0;

    if (sspcfgptr->init == TRUE)
    {
        sspregs = sspcfgptr->regptr;

        /* Loop until n_fifo expires to 0 */
        while (n_fifo > 0)
        {
            if ((sspregs->sr & SSP_SR_TNF) != 0)
            {
                /* Write data from buffer into FIFO */
                sspregs->dr = (UNS_32) *data;
                data++;

                /* Increment data count and decrement buffer size
                   count */
                count++;
                n_fifo--;
            }
        }
    }

    return count;
}

/***********************************************************************
 *
 * Function: ssp_read_ring
 *
 * Purpose: SSP ring buffer (interrupt) read function
 *
 * Processing:
 *     If the init flag for the SSP structure is FALSE, return 0 to
 *     the caller. Otherwise, save the state of the receive interrupt
 *     and disable the receive interrupt. Loop until "max_fifo" equals
 *     0 or until the receive ring buffer is empty, whichever comes
 *     first. Read the data from the ring buffer  indexed by the tail
 *     pointer and place it into the user buffer. Increment the tail
 *     pointer and user buffer pointer. If the tail pointer exceeds the
 *     buffer size, set the tail pointer to 0. Increment "count", and
 *     decrement "max_fifo". Exit the loop based on the loop conditions,
 *     re-enable the receive interrupts, and return the number of times
 *     the fifo was read to the caller.
 *
 * Parameters:
 *     devid:     Pointer to SSP config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_fifo:  Number of times to read receive fifo
 *
 * Outputs: None
 *
 * Returns: Number of times the receive fifo was actually read
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_read_ring(INT_32 devid,
                     void *buffer,
                     INT_32 max_fifo)
{
    SSP_REGS_T *sspregs;
    UNS_32 tmp1;
    SSP_CFG_T *sspcfgptr = (SSP_CFG_T *) devid;
    UNS_16 *data = (UNS_16 *) buffer;
    INT_32 count = 0;

    if (sspcfgptr->init == TRUE)
    {
        sspregs = sspcfgptr->regptr;

        /* Temporarily lock out SSP receive interrupts during this
           read so the SSP receive interrupt won't cause problems
           with the index values */
        tmp1 = sspregs->cr1 & SSP_CR1_RIE;
        sspregs->cr1 &= ~SSP_CR1_RIE;

        /* Loop until receive ring buffer is empty or until max_bytes
           expires */
        while ((max_fifo > 0) &&
            (sspcfgptr->rx_tail != sspcfgptr->rx_head))
        {
            /* Read data from ring buffer into user buffer */
            *data = sspcfgptr->rx[sspcfgptr->rx_tail];
            data++;

            /* Update tail pointer */
            sspcfgptr->rx_tail++;

            /* Make sure tail didn't overflow */
            if (sspcfgptr->rx_tail >= SSP_R_SIZE)
            {
                sspcfgptr->rx_tail = 0;
            }

            /* Increment data count and decrement buffer size count */
            count++;
            max_fifo--;
        }

        /* Re-enable SSP interrupts */
        sspregs->cr1 |= tmp1;
    }

    return count;
}

/***********************************************************************
 *
 * Function: ssp_write_ring
 *
 * Purpose: SSP ring buffer (interrupt) write function
 *
 * Processing:
 *     If the init flag for the SSP structure is FALSE, return 0 to
 *     the caller. Otherwise, disable the transmit interrupts. Loop
 *     until "n_fifo" equals 0 or until the transmit ring buffer is full,
 *     whichever comes first. Write the data from the user buffer to
 *     the transmit ring buffer indexed by the head pointer. Increment
 *     the user buffer pointer and head pointer. If the head pointer
 *     exceeds the buffer size, set the head pointer to 0. Increment
 *     "count", and decrement "n_fifo". Exit the loop based on the loop
 *     conditions. If the number of times the ring buffer was written is
 *     greater then 0, call the ssp_standard_transmit() function.
 *     Return the number of times the transmit fifo was written to the caller.
 *
 * Parameters:
 *     devid:   Pointer to SSP config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_fifo: Number of times to write data to the transmit fifo
 *
 * Outputs: None
 *
 * Returns: Number of times the transmit fifo was actually written
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ssp_write_ring(INT_32 devid,
                      void *buffer,
                      INT_32 n_fifo)
{
    SSP_REGS_T *sspregs;
    SSP_CFG_T *sspcfgptr = (SSP_CFG_T *) devid;
    UNS_16 *data = (UNS_16 *) buffer;
    INT_32 count = 0;

    if (sspcfgptr->init == TRUE)
    {
        sspregs = sspcfgptr->regptr;

        /* Temporarily lock out SSP transmit interrupts during this
           read so the SSP transmit interrupt won't cause problems
           with the index values */
        sspregs->cr1 &= ~SSP_CR1_TIE;

        /* Loop until transmit ring buffer is full or until n_bytes
           expires */
        while ((n_fifo > 0) &&
            (ssp_get_free_tx_count(sspcfgptr) > 0))
        {
            /* Write data from buffer into ring buffer */
            sspcfgptr->tx[sspcfgptr->tx_head] = *data;
            data++;

            /* Increment head pointer */
            sspcfgptr->tx_head++;
            if (sspcfgptr->tx_head >= SSP_R_SIZE)
            {
                sspcfgptr->tx_head = 0;
            }

            /* Increment data count and decrement buffer size count */
            count++;
            n_fifo--;
        }

        /* Now start sending the data - this will also re-enable the
           transmit interrupt */
        if (count > 0)
        {
            ssp_standard_transmit(sspcfgptr);
        }
    }

    return count;
}

/***********************************************************************
 *
 * Function: ssp_isr
 *
 * Purpose: Default SSP interrupt
 *
 * Processing:
 *     Call the standard SSP interrupt function
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
void ssp_isr(void)
{
    ssp_standard_interrupt(&sspcfg);
}
