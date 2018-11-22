/***********************************************************************
 * $Id:: lh7a404_uart_driver.c 14 2007-08-28 16:08:18Z kevinw          $
 *
 * Project: LH7A404 UART driver
 *
 * Description:
 *     This file contains driver support for the UART modules on the
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

#include "lh7a404_uart_driver.h"

/***********************************************************************
 * UART driver private data and types
 **********************************************************************/

/* UART Ring buffer structure */
typedef struct
{
    UNS_8  tx[UART_RING_BUFSIZE];  /* UART Tx data ring buffer */
    UNS_8  rx[UART_RING_BUFSIZE];  /* UART Rx data ring buffer */
    INT_32 tx_head;                /* UART Tx ring buffer head index */
    INT_32 tx_tail;                /* UART Tx ring buffer tail index */
    INT_32 rx_head;                /* UART Rx ring buffer head index */
    INT_32 rx_tail;                /* UART Rx ring buffer tail index */
} UART_RING_BUFFER_T;

/* UART device configuration structure type */
typedef struct
{
    BOOL_32 init;          /* Device initialized flag */
    UART_RING_BUFFER_T rb; /* Device ring buffer */
    UART_REGS_T *regptr;   /* Pointer to UART registers */
} UART_CFG_T;

/* UART device configuration structure for UART 1 */
#if UART_1_ENABLE == 1
STATIC UART_CFG_T uart1cfg;
#endif

/* UART device configuration structure for UART 2 */
#if UART_2_ENABLE == 1
STATIC UART_CFG_T uart2cfg;
#endif

/* UART device configuration structure for UART 3 */
#if UART_3_ENABLE == 1
STATIC UART_CFG_T uart3cfg;
#endif

/***********************************************************************
 * UART driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: uart_standard_receive
 *
 * Purpose: Common UART receive function
 *
 * Processing:
 *     While the UART receive FIFO not empty status is set, read a data
 *     value from the FIFO and place it into the receive ring buffer
 *     indexed by the head pointer. Increment the head pointer and reset
 *     it to the start of the ring buffer if it overflows. Continue
 *     until the FIFO is empty.
 *
 * Parameters:
 *     uartcfg: Pointer to UART config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     There is no protection against overflowing the ring buffer.
 *
 **********************************************************************/
STATIC void uart_standard_receive(UART_CFG_T *uartcfg)
{
    UART_REGS_T *uartregs = uartcfg->regptr;

    /* Continue stuffing the receive ring FIFO until the receive FIFO
       is empty */
    while ((uartregs->status & UART_STATUS_RXFE) == 0)
    {
        uartcfg->rb.rx[uartcfg->rb.rx_head] =
            (UNS_8) (uartregs->data & UART_DATA_MASK);

        /* Increment receive ring buffer head pointer */
        uartcfg->rb.rx_head++;
        if (uartcfg->rb.rx_head >= UART_RING_BUFSIZE)
        {
            uartcfg->rb.rx_head = 0;
        }
    }
}

/***********************************************************************
 *
 * Function: uart_standard_transmit
 *
 * Purpose: Common UART transmit function
 *
 * Processing:
 *     While the UART transmit FIFO not full status is set and there is
 *     data to transmit in the ring buffer, write a data value from the
 *     ring buffer into the transmit FIFO. Increment the transmit ring
 *     buffer tail pointer and reset it to the start of the ring buffer
 *     if it overflows. Continue until the transmit FIFO is full or the
 *     ring buffer is empty. If all the data has been transmitted from
 *     the ring buffer, disable the transmit interrupt, otherwise
 *     (re)enable it.
 *
 * Parameters:
 *     uartcfg: Pointer to UART config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void uart_standard_transmit(UART_CFG_T *uartcfg)
{
    UART_REGS_T *uartregs = uartcfg->regptr;

    /* Continue stuffing the transmit FIFO until it is full, or until
       the ring buffers are empty */
    while ((uartcfg->rb.tx_head != uartcfg->rb.tx_tail)
        && ((uartregs->status & UART_STATUS_TXFF) == 0))
    {
        /* Move a piece of data into the transmit FIFO */
        uartregs->data = (UNS_32)
            uartcfg->rb.tx[uartcfg->rb.tx_tail];

        /* Update transmit ring FIFO tail pointer */
        uartcfg->rb.tx_tail++;
        if (uartcfg->rb.tx_tail >= UART_RING_BUFSIZE)
        {
            uartcfg->rb.tx_tail = 0;
        }
    }

    /* If there is no more data to send, disable the transmit
       interrupt - else enable it or keep it enabled */
    if (uartcfg->rb.tx_head == uartcfg->rb.tx_tail)
    {
        uartregs->inte &= ~UART_INTR_TI;
    }
    else
    {
        uartregs->inte |= UART_INTR_TI;
    }
}

/***********************************************************************
 *
 * Function: uart_standard_interrupt
 *
 * Purpose: Common UART interrupt function
 *
 * Processing:
 *     If the UART receive interrupt or receive timeout interrupt has
 *     occurred, call the uart_standard_receive function. If the UART
 *     transmit interrupt has occurred, call the uart_standard_transmit
 *     function. If the UART modem status interrupt has occurred,
 *     disable the modem status interrupt.
 *
 * Parameters:
 *     uartcfg: Pointer to UART config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void uart_standard_interrupt(UART_CFG_T *uartcfg)
{
    UART_REGS_T *uartregs = uartcfg->regptr;

    /* Interrupt was due to a receive data FIFO service request */
    if ((uartregs->intr & (UART_INTR_RTI | UART_INTR_RI)) != 0)
    {
        uart_standard_receive(uartcfg);
    }

    /* Interrupt was due to a transmit data FIFO service request */
    if ((uartregs->intr & UART_INTR_TI) != 0)
    {
        uart_standard_transmit(uartcfg);
    }

    /* Interrupt was due to a modem service request */
    if ((uartregs->intr & UART_INTR_MI) != 0)
    {
        /* Nothing to do here - add functionality as needed. For now,
           just disable the interrupt so it won't happen again */
        uartregs->inte &= ~UART_INTR_MI;
    }
}

/***********************************************************************
 *
 * Function: uart_get_free_tx_count
 *
 * Purpose: Return the amount of free space in the transmit ring buffer
 *
 * Processing:
 *     Compute a count value by subtracting the transmit ring buffer
 *     tail index from the head index. If the value is less than 0,
 *     then recompute the count value adding the head index to the
 *     total ring buffer size minus the tail index. Return the number
 *     of free entries left in the ring buffer by subtracting the
 *     (count + 1) from the total ring buffer size.
 *
 * Parameters:
 *     uartcfg: Pointer to UART config structure
 *
 * Outputs: None
 *
 * Returns: The number of free bytes in the transmit ring buffer
 *
 * Notes: None
 *
 **********************************************************************/
STATIC INT_32 uart_get_free_tx_count(UART_CFG_T *uartcfg)
{
    INT_32 count = 0;

    count = uartcfg->rb.tx_head - uartcfg->rb.tx_tail;
    if (count < 0)
    {
        /* head pointer has flipped to start of ring */
        count = (UART_RING_BUFSIZE - uartcfg->rb.tx_tail) +
            uartcfg->rb.tx_head;
    }

    return (UART_RING_BUFSIZE - count - 1);
}

/***********************************************************************
 * UART driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: uart_open
 *
 * Purpose: Open a UART
 *
 * Processing:
 *     Determine the UART configuration structure to use based on the
 *     paseed arg value. If the arg value doesn't match an available
 *     UART, return NULL to the caller. Otherwise, check the status of
 *     the init flag. If it is TRUE, return NULL to the caller.
 *     Otherwise, set init to TRUE and save the pointe to the UART
 *     registers. Disable the UART to clear the UART FIFOs. Re-enable
 *     the UART is UART mode (vs SIR mode). Disable UART interrupts.
 *     Set the receive and reansmit ring buffers to empty by setting
 *     the head and tail pointers to 0. Enable the UART is 9600-N-8-1
 *     mode with FIFOs enabled and return the pointer to the UART
 *     config structure to the caller.
 *
 * Parameters:
 *     ipbase: UART descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a UART config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_open(void *ipbase, INT_32 arg)
{
    UART_CFG_T *uartcfg;

    /* Invalid or disabled UART */
    uartcfg = (UART_CFG_T *) NULL;

#if UART_1_ENABLE == 1
    if ((UART_REGS_T *) ipbase == UART1)
    {
        /* UART 1 selected */
        uartcfg = &uart1cfg;
    }
#endif

#if UART_2_ENABLE == 1
    if ((UART_REGS_T *) ipbase == UART2)
    {
        /* UART 2 selected */
        uartcfg = &uart2cfg;
    }
#endif

#if UART_3_ENABLE == 1
    if ((UART_REGS_T *) ipbase == UART3)
    {
        /* UART 3 selected */
        uartcfg = &uart3cfg;
    }
#endif

    if (uartcfg != (UART_CFG_T *) NULL)
    {
        /* Valid UART selected, has it been previously initialized? */
        if (uartcfg->init == FALSE)
        {
            /* Device not initialized and it is usable, so set it to
               used */
            uartcfg->init = TRUE;

            /* Save address of register block */
            uartcfg->regptr = (UART_REGS_T *) ipbase;

            /* The UART is disabled prior to enabling so the FIFOs are
               empty when it is re-enabled */
            uartcfg->regptr->control = UART_CNTL_SIR_DIS;

            /* Enable the UART - all UART registers are reliable only
               then the UART is enabled */
            uartcfg->regptr->control =
                (UART_CNTL_EN | UART_CNTL_SIR_DIS);

            /* Disable interrupts - the interrupts remain off until
                they are enabled */
            uartcfg->regptr->inte = 0;

            /* Initialize the ring buffers */
            uartcfg->rb.tx_head = uartcfg->rb.tx_tail = 0;
            uartcfg->rb.rx_head = uartcfg->rb.rx_tail = 0;

            /* Set default UART configuration to FIFO enabled, 1 stop
               bit, no parity, and 8 data bits */
            uartcfg->regptr->lcr = (UART_LCR_WLEN8 | UART_LCR_FEN |
                UART_LCR_STP1 | UART_LCR_PNONE);

            /* Set default baud rate to 9600bps */
            uartcfg->regptr->bcr = (UNS_32) UART_BCR_9600;
        }
    }

    return (INT_32) uartcfg;
}

/***********************************************************************
 *
 * Function: uart_close
 *
 * Purpose: Close a UART
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the UART,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to UART config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS uart_close(INT_32 devid)
{
    UART_CFG_T *uartcfgptr = (UART_CFG_T *) devid;
    STATUS status = _ERROR;

    if (uartcfgptr->init == TRUE)
    {
        /* Disable UART */
        uartcfgptr->regptr->control = UART_CNTL_SIR_DIS;

        status = _NO_ERROR;
        uartcfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: uart_ioctl
 *
 * Purpose: UART configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate UART parameter.
 *
 * Parameters:
 *     devid: Pointer to UART config structure
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
STATUS uart_ioctl(INT_32 devid,
                  INT_32 cmd,
                  INT_32 arg)
{
    UNS_32 tmp1;
    UART_REGS_T *uartregs;
    UART_CFG_T *uartcfgptr = (UART_CFG_T *) devid;
    STATUS status = _ERROR;

    if (uartcfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        uartregs = uartcfgptr->regptr;

        switch (cmd)
        {
            case UART_ENABLE:
                /* Enable or disable the UART */
                if ((INT_32) arg == 1)
                {
                    /* Enable UART */
                    uartregs->control |= UART_CNTL_EN;
                }
                else
                {
                    /* Disable UART */
                    uartregs->control &= ~UART_CNTL_EN;
                }
                break;

            case SIR_ENABLE:
                /* Enable or disable UART SIR mode */
                if (uartregs == UART1)
                {
                    /* Only valud on UART 1 */
                    if ((INT_32) arg == 1)
                    {
                        /* Enable UART SIR */
                        uartregs->control &= ~UART_CNTL_SIR_DIS;
                    }
                    else
                    {
                        /* Disable UART SIR */
                        uartregs->control |= UART_CNTL_SIR_DIS;
                    }
                }
                else
                {
                    /* Not valid on other UARTS */
                    status = SMA_BAD_PARAMS;
                }
                break;

            case UART_SET_BAUD_RATE:
                /* Set the bps rate */
                switch (arg)
                {
                    case BPS_2400:
                        uartregs->bcr = (UNS_32) UART_BCR_2400;
                        break;

                    case BPS_4800:
                        uartregs->bcr = (UNS_32) UART_BCR_4800;
                        break;

                    case BPS_9600:
                        uartregs->bcr = (UNS_32) UART_BCR_9600;
                        break;

                    case BPS_19200:
                        uartregs->bcr = (UNS_32) UART_BCR_19200;
                        break;

                    case BPS_28800:
                        uartregs->bcr = (UNS_32) UART_BCR_28800;
                        break;

                    case BPS_38400:
                        uartregs->bcr = (UNS_32) UART_BCR_38400;
                        break;

                    case BPS_57600:
                        uartregs->bcr = (UNS_32) UART_BCR_57600;
                        break;

                    case BPS_115200:
                        uartregs->bcr = (UNS_32) UART_BCR_115200;
                        break;

                    case BPS_153600:
                        uartregs->bcr = (UNS_32) UART_BCR_153600;
                        break;

                    case BPS_230400:
                        uartregs->bcr = (UNS_32) UART_BCR_230400;
                        break;

                    case BPS_460800:
                        uartregs->bcr = (UNS_32) UART_BCR_460800;
                        break;

                    case BPS_921600:
                        uartregs->bcr = (UNS_32) UART_BCR_921600;
                        break;
                        
                    default:
                        /* Baud bps rate */
                        status = SMA_BAD_PARAMS;
                }
                break;

            case UART_SET_DATA_BITS:
                /* Set the data bit width */
                if ((arg >= 5) && (arg <= 8))
                {
                    tmp1 = uartregs->lcr & ~UART_LCR_WLEN8;
                    uartregs->lcr = (tmp1 | UART_LCR_WLEN(arg));
                }
                else
                {
                    /* Baud data width */
                    status = SMA_BAD_PARAMS;
                }
                break;

            case UART_SET_PARITY:
                /* Set UART parity */
                switch (arg)
                {
                    case UART_PARITY_NONE:
                        /* Disable parity */
                        uartregs->lcr &= ~UART_LCR_PEN;
                        break;

                    case UART_PARITY_ODD:
                        /* Odd parity */
                        uartregs->lcr |= UART_LCR_PEN;
                        uartregs->lcr &= ~UART_LCR_PEVEN;
                        break;

                    case UART_PARITY_EVEN:
                        /* Even parity */
                        uartregs->lcr |= (UART_LCR_PEN | UART_LCR_PEVEN);
                        break;
                    
                    default:
                        /* Baud parity */
                        status = SMA_BAD_PARAMS;
                }
                break;

            case UART_SET_STOP_BITS:
                /* Set the number of stop bits */
                if (arg == 1)
                {
                    /* 1 stop bit */
                    uartregs->lcr &= ~UART_LCR_STP2;
                }
                else if (arg == 2)
                {
                    /* 2 stop bits */
                    uartregs->lcr |= UART_LCR_STP2;
                }
                else
                {
                    /* Baud data width */
                    status = SMA_BAD_PARAMS;
                }
                break;

            case UART_ENABLE_FIFO:
                /* Enable or disable the FIFOs */
                if ((INT_32) arg == 1)
                {
                    /* Enable FIFOs */
                    uartregs->lcr |= UART_LCR_FEN;
                }
                else
                {
                    /* Disable FIFOs */
                    uartregs->lcr &= ~UART_LCR_FEN;
                }
                break;

            case UART_ENABLE_INTS:
                /* Enable selected interrupts */
                tmp1 = ((UNS_32) arg | uartregs->inte) &
                    (UART_INTR_RTI | UART_INTR_MI | UART_INTR_TI |
                    UART_INTR_RI | UART_INTR_OVI | UART_INTR_BEI |
                    UART_INTR_PEI | UART_INTR_FEI);
                uartregs->inte = tmp1;
                break;

            case UART_DISABLE_INTS:
                /* Disable selected interrupts */
                tmp1 = (UNS_32) arg & (UART_INTR_RTI | UART_INTR_MI |
                    UART_INTR_TI | UART_INTR_RI | UART_INTR_OVI |
                    UART_INTR_BEI | UART_INTR_PEI | UART_INTR_FEI);
                uartregs->inte &= ~tmp1;
                break;

            case UART_ASSERT_BREAK:
                /* Assert or de-assert a break */
                if (arg == 1)
                {
                    /* Assert break */
                    uartregs->lcr |= UART_LCR_SENDBRK;
                }
                else
                {
                    /* De-assert break */
                    uartregs->lcr &= ~UART_LCR_SENDBRK;
                }
                break;

            case UART_SET_LOOPBACK:
                /* Enable or disable loopback mode */
                if (arg == 1)
                {
                    /* Enable loopback mode */
                    uartregs->control |= UART_CNTL_LBE;
                }
                else
                {
                    /* Disable loopback mode */
                    uartregs->control &= ~UART_CNTL_LBE;
                }
                break;

            case UART_SET_MDM_POL:
                /* Set modem signal polarity */
                if (arg == 1)
                {
                    /* Set modem signal active high */
                    uartregs->control |= UART_CNTL_MXP;
                }
                else
                {
                    /* Set modem signal active low */
                    uartregs->control &= ~UART_CNTL_MXP;
                }
                break;

            case UART_SET_TX_POL:
                /* Set TX signal polarity */
                if (arg == 1)
                {
                    /* Set TX active high */
                    uartregs->control |= UART_CNTL_TXP;
                }
                else
                {
                    /* Set TX active low */
                    uartregs->control &= ~UART_CNTL_TXP;
                }
                break;

            case UART_SET_RX_POL:
                /* Set RX signal polarity */
                if (arg == 1)
                {
                    /* Set RX active low */
                    uartregs->control |= UART_CNTL_RXP;
                }
                else
                {
                    /* Set RX active high */
                    uartregs->control &= ~UART_CNTL_RXP;
                }
                break;

            case SIR_SET_BLANKING:
                /* Enable or disable SIR blanking */
                if (arg == 1)
                {
                    /* Disable SIR blanking */
                    uartregs->control |= UART_CNTL_SIRBD;
                }
                else
                {
                    /* Enable SIR blanking */
                    uartregs->control &= ~UART_CNTL_SIRBD;
                }
                break;

            case SIR_SET_LOWPOWER:
                /* Enables or disables SIR low power mode */
                if (arg == 1)
                {
                    /* Enable SIR low power mode */
                    uartregs->control |= UART_CNTL_SIRLP;
                }
                else
                {
                    /* Disable SIR low power mode */
                    uartregs->control &= ~UART_CNTL_SIRLP;
                }
                break;

            case UART_GET_STATUS:
                /* Return a UART status */
                switch (arg)
                {
                    case UART_ENABLE_ST:
                        /* Return UART enabled status */
                        if ((uartregs->control & UART_CNTL_EN) != 0)
                        {
                            /* UART is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* UART is disabled */
                            status = 0;
                        }
                        break;

                    case SIR_ENABLE_ST:
                        /* Return UART SIR enabled status */
                        if ((uartregs->control &
                            UART_CNTL_SIR_DIS) != 0)
                        {
                            /* UART SIR is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* UART SIR  is disabled */
                            status = 0;
                        }
                        break;

                    case UART_BAUD_RATE:
                        /* Return the baud rate enumeration */
                        switch (uartregs->bcr)
                        {
                            case UART_BCR_2400:
                                status = BPS_2400;
                                break;

                            case UART_BCR_4800:
                                status = BPS_4800;
                                break;

                            case UART_BCR_9600:
                                status = BPS_9600;
                                break;

                            case UART_BCR_19200:
                                status = BPS_19200;
                                break;

                            case UART_BCR_28800:
                                status = BPS_28800;
                                break;

                            case UART_BCR_38400:
                                status = BPS_38400;
                                break;

                            case UART_BCR_57600:
                                status = BPS_57600;
                                break;

                            case UART_BCR_115200:
                                status = BPS_115200;
                                break;

                            case UART_BCR_153600:
                                status = BPS_153600;
                                break;

                            case UART_BCR_230400:
                                status = BPS_230400;
                                break;

                            case UART_BCR_460800:
                                status = BPS_460800;
                                break;

                            default:
                                /* Unknown bps rate */
                                status = _ERROR;
                                break;
                        }
                        break;

                    case UART_DATA_BITS:
                        /* Return the number of data bits (5 to 8) */
                        switch (uartregs->lcr & UART_LCR_WLEN8)
                        {
                            case UART_LCR_WLEN5:
                                status = 5;
                                break;

                            case UART_LCR_WLEN6:
                                status = 6;
                                break;

                            case UART_LCR_WLEN7:
                                status = 7;
                                break;

                            case UART_LCR_WLEN8:
                            default:
                                status = 8;
                                break;
                        }
                        break;

                    case UART_PARITY:
                        /* Return selected parity */
                        if ((uartregs->lcr & UART_LCR_PEN) != 0)
                        {
                            /* Parity is enabled */
                            if ((uartregs->lcr & UART_LCR_PEVEN) != 0)
                            {
                                /* Even parity */
                                status = UART_PARITY_EVEN;
                            }
                            else
                            {
                                /* Odd parity */
                                status = UART_PARITY_ODD;
                            }
                        }
                        else
                        {
                            /* No parity */
                            status = UART_PARITY_NONE;
                        }
                        break;

                    case UART_STOP_BITS:
                        /* Return the number of stop bits */
                        if ((uartregs->lcr & UART_LCR_STP2) != 0)
                        {
                            /* 2 stop bits */
                            status = 2;
                        }
                        else
                        {
                            status = 1;
                        }
                        break;

                    case UART_INT_ST:
                        /* Return enabled interrupts */
                        status = uartregs->inte & (UART_INTR_RTI |
                            UART_INTR_MI | UART_INTR_TI | UART_INTR_RI |
                            UART_INTR_OVI | UART_INTR_BEI |
                            UART_INTR_PEI | UART_INTR_FEI);
                        break;

                    case UART_LOOPBACK_ST:
                        /* Returns Loopback mode enabled status */
                        if ((uartregs->lcr & UART_CNTL_LBE) != 0)
                        {
                            /* Loopback mode is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* Loopback mode is disabled */
                            status = 0;
                        }
                        break;

                    case UART_MDM_POL_ST:
                        /* Returns modem polarity bit status */
                        if ((uartregs->lcr & UART_CNTL_MXP) != 0)
                        {
                            /* Active high */
                            status = 1;
                        }
                        else
                        {
                            /* Active low */
                            status = 0;
                        }
                        break;

                    case UART_TX_POL_ST:
                        /* Returns TX signal polarity status */
                        if ((uartregs->lcr & UART_CNTL_TXP) != 0)
                        {
                            /* Active high */
                            status = 1;
                        }
                        else
                        {
                            /* Active low */
                            status = 0;
                        }
                        break;

                    case UART_RX_POL_ST:
                        /* Returns RX signal polarity status */
                        if ((uartregs->lcr & UART_CNTL_RXP) != 0)
                        {
                            /* Active low */
                            status = 1;
                        }
                        else
                        {
                            /* Active high */
                            status = 0;
                        }
                        break;

                    case SIR_BLANKING_ST:
                        /* Returns SIR blanking bit status */
                        if ((uartregs->lcr & UART_CNTL_SIRBD) != 0)
                        {
                            /* SIR blanking disabled */
                            status = 1;
                        }
                        else
                        {
                            /* SIR blanking enabled */
                            status = 0;
                        }
                        break;

                    case SIR_LOWPOWER_ST:
                        /* Returns SIR low power mode status */
                        if ((uartregs->lcr & UART_CNTL_SIRLP) != 0)
                        {
                            /* SIR low power mode enabled */
                            status = 1;
                        }
                        else
                        {
                            /* SIR low power mode disabled */
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
 * Function: uart_read_polled
 *
 * Purpose: UART read function for polled mode
 *
 * Processing:
 *     If the init flag for the UART structure is FALSE, return 0 to
 *     the caller. Otherwise, loop until max_bytes equals 0 or until
 *     the receive FIFO is empty, whichever comes first. Read the data
 *     from the UART FIFO and place it user buffer. Increment the
 *     address of the user buffer. Increment bytes, and decrement
 *     max_bytes. Exit the loop based on the loop conditions and
 *     return the number of bytes read to the caller.
 *
 * Parameters:
 *     devid:     Pointer to UART config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read from the FIFO
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_read_polled(INT_32 devid,
                        void *buffer,
                        INT_32 max_bytes)
{
    UART_REGS_T *uartregs;
    UART_CFG_T *uartcfgptr = (UART_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    INT_32 bytes = 0;

    if (uartcfgptr->init == TRUE)
    {
        uartregs = uartcfgptr->regptr;

        /* Loop until receive FIFO is empty or until max_bytes
           expires */
        while ((max_bytes > 0) &&
            ((uartregs->status & UART_STATUS_RXFE) == 0))
        {
            /* Read data from FIFO into buffer */
            *data = (UNS_8) uartregs->data;
            data++;

            /* Increment data count and decrement buffer size count */
            bytes++;
            max_bytes--;
        }
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: uart_write_polled
 *
 * Purpose: UART write function for polled mode
 *
 * Processing:
 *     If the init flag for the UART structure is FALSE, return 0 to
 *     the caller. Otherwise, loop until n_bytes equals 0. Loop until
 *     the transmit FIFO is full insisde the n_bytes loop. Read the data
 *     from the user buffer and place it in the UART FIFO. Increment
 *     the address of the user buffer. Increment bytes, and decrement
 *     n_bytes. Exit the loop based on the loop conditions and
 *     return the number of actually bytes written to the caller.
 *
 * Parameters:
 *     devid:   Pointer to UART config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written to the FIFO
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_write_polled(INT_32 devid,
                         void *buffer,
                         INT_32 n_bytes)
{
    UART_REGS_T *uartregs;
    UART_CFG_T *uartcfgptr = (UART_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    INT_32 bytes = 0;

    if (uartcfgptr->init == TRUE)
    {
        uartregs = uartcfgptr->regptr;

        /* Loop until n_bytes expires to 0 */
        while (n_bytes > 0)
        {
            if ((uartregs->status & UART_STATUS_TXFF) == 0)
            {
                /* Write data from buffer into FIFO */
                uartregs->data = *data;
                data++;

                /* Increment data count and decrement buffer size
                   count */
                bytes++;
                n_bytes--;
            }
        }
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: uart_read_ring
 *
 * Purpose: UART read function for interrupt mode (using ring buffers)
 *
 * Processing:
 *     If the init flag for the UART structure is FALSE, return 0 to
 *     the caller. Otherwise, save the state of the receive interrupts
 *     and disable the receive interrupts. Loop until max_bytes equals
 *     0 or until the receive ring buffer is empty, whichever comes
 *     first. Read the data from the ring buffer  indexed by the tail
 *     pointer and place it into the user buffer. Increment the tail
 *     pointer and user buffer pointer. If the tail pointer exceeds the
 *     buffer size, set the tail pointer to 0. Increment bytes, and
 *     decrement max_bytes. Exit the loop based on the loop conditions,
 *     re-enable the receive interrupts, and return the number of bytes
 *     read to the caller.
 *
 * Parameters:
 *     devid:     Pointer to UART config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read from the ring buffer
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 uart_read_ring(INT_32 devid,
                      void *buffer,
                      INT_32 max_bytes)
{
    UART_REGS_T *uartregs;
    UNS_32 tmp1;
    UART_CFG_T *uartcfgptr = (UART_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    INT_32 bytes = 0;

    if (uartcfgptr->init == TRUE)
    {
        uartregs = uartcfgptr->regptr;

        /* Temporarily lock out UART receive interrupts during this
           read so the UART receive interrupt won't cause problems
           with the index values */
        tmp1 = uartregs->inte & (UART_INTR_RTI | UART_INTR_RI);
        uartregs->inte &= ~(UART_INTR_RTI | UART_INTR_RI);

        /* Loop until receive FIFO is empty or until max_bytes
           expires */
        while ((max_bytes > 0) &&
            (uartcfgptr->rb.rx_tail != uartcfgptr->rb.rx_head))
        {
            /* Read data from ring buffer into user buffer */
            *data = uartcfgptr->rb.rx[uartcfgptr->rb.rx_tail];
            data++;

            /* Update tail pointer */
            uartcfgptr->rb.rx_tail++;

            /* Make sure tail didn't overflow */
            if (uartcfgptr->rb.rx_tail >= UART_RING_BUFSIZE)
            {
                uartcfgptr->rb.rx_tail = 0;
            }

            /* Increment data count and decrement buffer size count */
            bytes++;
            max_bytes--;
        }

        /* Re-enable UART interrupts */
        uartregs->inte |= tmp1;
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: uart_write_ring
 *
 * Purpose: UART write function for interrupt mode (using ring buffers)
 *
 * Processing:
 *     If the init flag for the UART structure is FALSE, return 0 to
 *     the caller. Otherwise, disable the receive interrupts. Loop until
 *     n_bytes equals 0 or until the transmit ring buffer is full,
 *     whichever comes first. Write the data from the user buffer to
 *     the transmit ring buffer indexed by the head pointer. Increment
 *     the user buffer pointer and head pointer. If the head pointer
 *     exceeds the buffer size, set the head pointer to 0. Increment
 *     bytes, and decrement n_bytes. Exit the loop based on the loop
 *     conditions. If the number bytes written to the ring buffer was
 *     greater then 0, call the uart_standard_transmit() fucntion.
 *     Return the number of bytes read to the caller.
 *
 * Parameters:
 *     devid:   Pointer to UART config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written to the ring buffer
 *
 * Notes:
 *     You cannot overflow the transmit ring buffer.
 *
 **********************************************************************/
INT_32 uart_write_ring(INT_32 devid,
                       void *buffer,
                       INT_32 n_bytes)
{
    UART_REGS_T *uartregs;
    UNS_32 savedint;
    UART_CFG_T *uartcfgptr = (UART_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    INT_32 bytes = 0;

    if (uartcfgptr->init == TRUE)
    {
        uartregs = uartcfgptr->regptr;

        /* Temporarily lock out UART transmit interrupts during this
           read so the UART transmit interrupt won't cause problems
           with the index values */
        savedint = uartregs->inte & UART_INTR_TI;
        uartregs->inte &= ~UART_INTR_TI;

        /* Loop until transmit run buffer is full or until n_bytes
           expires */
        while ((n_bytes > 0) &&
            (uart_get_free_tx_count(uartcfgptr) > 0))
        {
            /* Write data from buffer into ring buffer */
            uartcfgptr->rb.tx[uartcfgptr->rb.tx_head] = *data;
            data++;

            /* Increment head pointer */
            uartcfgptr->rb.tx_head++;
            if (uartcfgptr->rb.tx_head >= UART_RING_BUFSIZE)
            {
                uartcfgptr->rb.tx_head = 0;
            }

            /* Increment data count and decrement buffer size count */
            bytes++;
            n_bytes--;
        }

        /* Now start sending the data - this will also re-enable the
           transmit interrupt */
        if (bytes > 0)
        {
            uart_standard_transmit(uartcfgptr);
        }
        else
        {
            /* Restore original interrupt state */
            uartregs->inte |= savedint;
        }
    }

    return bytes;
}

#if UART_1_ENABLE == 1
/***********************************************************************
 *
 * Function: uart1_isr
 *
 * Purpose: UART 1 interrupt function
 *
 * Processing:
 *     Calls the standard UART interrupt branch function with the
 *     UART1 configuration structure pointer.
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
void uart1_isr(void)
{
    uart_standard_interrupt(&uart1cfg);
}
#endif

#if UART_2_ENABLE == 1
/***********************************************************************
 *
 * Function: uart2_isr
 *
 * Purpose: UART 2 interrupt function
 *
 * Processing:
 *     Calls the standard UART interrupt branch function with the
 *     UART2 configuration structure pointer.
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
void uart2_isr(void)
{
    uart_standard_interrupt(&uart2cfg);
}
#endif

#if UART_3_ENABLE == 1
/***********************************************************************
 *
 * Function: uart3_isr
 *
 * Purpose: UART 3 interrupt function
 *
 * Processing:
 *     Calls the standard UART interrupt branch function with the
 *     UART3 configuration structure pointer.
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
void uart3_isr(void)
{
    uart_standard_interrupt(&uart3cfg);
}
#endif
