/***********************************************************************
 * $Id:: lh7a404_kmi_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 KMI driver
 *
 * Description:
 *     This file contains driver support for the KMI modules on the
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

#include "lh7a404_kmi_driver.h"
#include "lh7a404_csc_driver.h"

/***********************************************************************
 * KMI driver private data and types
 **********************************************************************/

/* Size of the KMI receive and transmit ring buffers */
#define KMI_RING_BUFSIZE 64

/* Required KMI reference clock speed in Hz */
#define KMIREFCLK 8000000

/* KMI Ring buffer structure */
typedef struct
{
    UNS_8  tx[KMI_RING_BUFSIZE];   /* KMI Tx data ring buffer */
    UNS_8  rx[KMI_RING_BUFSIZE];   /* KMI Rx data ring buffer */
    INT_32 tx_head;                /* KMI Tx ring buffer head index */
    INT_32 tx_tail;                /* KMI Tx ring buffer tail index */
    INT_32 rx_head;                /* KMI Rx ring buffer head index */
    INT_32 rx_tail;                /* KMI Rx ring buffer tail index */
} KMI_RING_BUFFER_T;

/* KMI device configuration structure type */
typedef struct
{
    BOOL_32 init;          /* Device initialized flag */
    KMI_RING_BUFFER_T rb;  /* Device ring buffer */
    KMI_REGS_T *regptr;    /* Pointer to KMI registers */
} KMI_CFG_T;

/* KMI device configuration structure */
STATIC KMI_CFG_T kmicfg;

/***********************************************************************
 * KMI driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: kmi_receive_int
 *
 * Purpose: KMI interrupt receive function
 *
 * Processing:
 *     While the KMI receive register is not empty, read a data value
 *     from the register and place it into the receive ring buffer
 *     indexed by the head pointer. Increment the head pointer and reset
 *     it to the start of the ring buffer if it overflows. Continue
 *     until the register is empty.
 *
 * Parameters:
 *     kmicfg: Pointer to KMI config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     There is no protection against overflowing the ring buffer.
 *
 **********************************************************************/
STATIC void kmi_receive_int(KMI_CFG_T *kmicfg)
{
    KMI_REGS_T *kmiregs = kmicfg->regptr;

    /* Continue stuffing the receive ring FIFO until the receive
       register is empty */
    while ((kmiregs ->mkistatus & KMI_RX_REG_FULL) != 0)
    {
        kmicfg->rb.rx[kmicfg->rb.rx_head] = (UNS_8) kmiregs->mkidata;

        /* Increment receive ring buffer head pointer */
        kmicfg->rb.rx_head++;
        if (kmicfg->rb.rx_head >= KMI_RING_BUFSIZE)
        {
            kmicfg->rb.rx_head = 0;
        }
    }
}

/***********************************************************************
 *
 * Function: kmi_transmit_int
 *
 * Purpose: KMI transmit function
 *
 * Processing:
 *     While the KMI transmit register is empty and there is data to
 *     transmit in the ring buffer, write a data value from the ring
 *     buffer into the transmit register. Increment the transmit ring
 *     buffer tail pointer and reset it to the start of the ring buffer
 *     if it overflows. Continue until the transmit register is full or
 *     the ring buffer is empty. If all the data has been transmitted
 *     from the ring buffer, disable the transmit interrupt, otherwise
 *     (re)enable it.
 *
 * Parameters:
 *     kmicfg: Pointer to KMI config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void kmi_transmit_int(KMI_CFG_T *kmicfg)
{
    KMI_REGS_T *kmiregs = kmicfg->regptr;

    /* Continue stuffing the transmit register until it is full, or
       until the ring buffers are empty */
    while ((kmicfg->rb.tx_head != kmicfg->rb.tx_tail)
        && ((kmiregs->mkistatus & KMI_TX_REG_EMPTY) != 0))
    {
        /* Move a piece of data into the transmit register */
        kmiregs->mkidata = (UNS_32)
            kmicfg->rb.tx[kmicfg->rb.tx_tail];

        /* Update transmit ring buffer tail pointer */
        kmicfg->rb.tx_tail++;
        if (kmicfg->rb.tx_tail >= KMI_RING_BUFSIZE)
        {
            kmicfg->rb.tx_tail = 0;
        }
    }

    /* If there is no more data to send, disable the transmit
       interrupt - else enable it or keep it enabled */
    if (kmicfg->rb.tx_head == kmicfg->rb.tx_tail)
    {
        kmiregs->mkictrl &= ~KMI_TXINT_EN;
    }
    else
    {
        kmiregs->mkictrl |= KMI_TXINT_EN;
    }
}

/***********************************************************************
 *
 * Function: kmi_get_free_tx_count
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
 *     kmicfg: Pointer to KMI config structure
 *
 * Outputs: None
 *
 * Returns: The number of free bytes in the transmit ring buffer
 *
 * Notes: None
 *
 **********************************************************************/
STATIC INT_32 kmi_get_free_tx_count(KMI_CFG_T *kmicfg)
{
    INT_32 count = 0;

    count = kmicfg->rb.tx_head - kmicfg->rb.tx_tail;
    if (count < 0)
    {
        /* head pointer has flipped to start of ring */
        count = (KMI_RING_BUFSIZE - kmicfg->rb.tx_tail) +
            kmicfg->rb.tx_head;
    }

    return (KMI_RING_BUFSIZE - count - 1);
}

/***********************************************************************
 * KMI driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: kmi_open
 *
 * Purpose: Open the KMI interface
 *
 * Processing:
 *     If the ipbase value doesn't match an available KMI address,
 *     return NULL to the caller. Otherwise, check the status of
 *     the init flag. If it is TRUE, return NULL to the caller.
 *     Otherwise, set init to TRUE and save the pointer to the KMI
 *     registers. Set the default KMI clock divider to 16. Enable the
 *     KMI interface without interrupts. Clear out the head and tail
 *     pointers of the KMI ring buffers. Return the address of the
 *     KMI configuration structure to the caller.
 *
 * Parameters:
 *     ipbase: KMI descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a KMI config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 kmi_open(void *ipbase,
                INT_32 arg)
{
    INT_32 status = 0;

    if (KMI == (KMI_REGS_T *) ipbase)
    {
        /* Valid KMI selected, has it been previously initialized? */
        if (kmicfg.init == FALSE)
        {
            /* Device not initialized and it is usable, so set it to
               used */
            kmicfg.init = TRUE;

            /* Save address of register block */
            kmicfg.regptr = (KMI_REGS_T *) ipbase;

            /* Setup KMI reference clock */
            kmi_ioctl((INT_32) &kmicfg, KMI_UPDATE_DIVIDER, 0);

            /* Set default state of KMI interface to enabled with no
               interrupts enabled */
            kmicfg.regptr->mkictrl = KMI_ENABLE;

            /* Initialize the ring buffers */
            kmicfg.rb.tx_head = kmicfg.rb.tx_tail = 0;
            kmicfg.rb.rx_head = kmicfg.rb.rx_tail = 0;

            status = (INT_32) &kmicfg;
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: kmi_close
 *
 * Purpose: Close the KMI interface
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the KMI,
 *     set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to KMI config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS kmi_close(INT_32 devid)
{
    KMI_CFG_T *kmicfgptr = (KMI_CFG_T *) devid;
    STATUS status = _ERROR;

    if (kmicfgptr->init == TRUE)
    {
        /* Disable KMI interface */
        kmicfgptr->regptr->mkictrl &= ~KMI_ENABLE;

        status = _NO_ERROR;
        kmicfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: kmi_ioctl
 *
 * Purpose: KMI configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate KMI parameter.
 *
 * Parameters:
 *     devid: Pointer to KMI config structure
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
STATUS kmi_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    UNS_32 tmp1;
    KMI_REGS_T *kmiregs;
    KMI_CFG_T *kmicfgptr = (KMI_CFG_T *) devid;
    STATUS status = _ERROR;

    if (kmicfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        kmiregs = kmicfgptr->regptr;

        switch (cmd)
        {
            case KMI_ENABLE_IF:
                /* Enable or disable the KMI */
                if (arg == 1)
                {
                    /* Enable KMI */
                    kmiregs->mkictrl |= KMI_ENABLE;
                }
                else
                {
                    /* Disable KMI */
                    kmiregs->mkictrl &= ~KMI_ENABLE;
                }
                break;

            case KMI_FORCE_CLK_LOW:
                /* Set KMI clock to normal mode or force low */
                if (arg == 1)
                {
                    /* Set clock signal low */
                    kmiregs->mkictrl |= KMI_FCLOCK_LOW;
                }
                else
                {
                    /* Set clock signal normal */
                    kmiregs->mkictrl &= ~KMI_FCLOCK_LOW;
                }
                break;

            case KMI_FORCE_DAT_LOW:
                /* Set KMI data to normal mode or force low */
                if (arg == 1)
                {
                    /* Set data signal low */
                    kmiregs->mkictrl |= KMI_FDATA_LOW;
                }
                else
                {
                    /* Set data signal normal */
                    kmiregs->mkictrl &= ~KMI_FDATA_LOW;
                }
                break;

            case KMI_SET_PS2_KB:
                /* Disable or enable PS2/AT keyboard interface */
                if (arg == 1)
                {
                    /* Select PS2/AT line control mode */
                    kmiregs->mkictrl |= KMI_NLC_TYPE;
                }
                else
                {
                    /* Select no line control mode */
                    kmiregs->mkictrl &= ~KMI_NLC_TYPE;
                }
                break;

            case KMI_ENABLE_INTS:
                /* Enable KMI interrupts */
                /* Mask off all bits except interrupt bits */
                tmp1 = kmiregs->mkictrl &
                    ~(KMI_RXINT_EN | KMI_TXINT_EN);

                /* OR in new selected enables */
                kmiregs->mkictrl = (tmp1 | (UNS_32) arg);
                break;

            case KMI_DISABLE_INTS:
                /* Disable KMI interrupts */
                /* Mask off selected bits */
                kmiregs->mkictrl &= ~((UNS_32) arg);
                break;

            case KMI_UPDATE_DIVIDER:
                /* Sets the KMI clock divider */
                /* Get bus speed needed to set reference KMI clock */
                tmp1 = csc_get_clock(AHBCLK);

                /* Set default speed of KMI reference clock as close as
                   possible to KMIREFCLK */
                tmp1 = csc_compute_divider(tmp1, KMIREFCLK);
                kmiregs->mkiclkdiv = KMI_CLKDIV(tmp1 - 1);
                break;
                
            case KMI_GET_STATUS:
                /* Return a KMI status */
                switch (arg)
                {
                    case KMI_ENABLED_ST:
                        /* Return KMI enabled status */
                        if ((kmiregs->mkictrl & KMI_ENABLE) != 0)
                        {
                            /* KMI is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* KMI is disabled */
                            status = 0;
                        }
                        break;

                    case KMI_TX_BUSY_ST:
                        /* Returns transmitter busy status */
                        if ((kmiregs->mkistatus & KMI_TX_BUSY) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;

                    case KMI_RX_BUSY_ST:
                        /* Returns receiver busy status */
                        if ((kmiregs->mkistatus & KMI_RX_BUSY) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;

                    case KMI_CLK_ST:
                        /* Returns state of clock line */
                        if ((kmiregs->mkistatus & KMI_CLK_ST_MSK) != 0)
                        {
                            status = 1;
                        }
                        else
                        {
                            status = 0;
                        }
                        break;

                    case KMI_DATA_ST:
                        /* Returns state of data line */
                        if ((kmiregs->mkistatus & KMI_DATA_ST_MSK) != 0)
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
 * Function: kmi_read_polled
 *
 * Purpose: KMI read function for polled mode
 *
 * Processing:
 *     If the init flag for the KMI structure is FALSE, return 0 to
 *     the caller. Otherwise, loop until max_bytes equals 0 or until
 *     the receive register is empty, whichever comes first. Read the
 *     data from the KMI receive register and place it user buffer.
 *     Increment the address of the user buffer. Increment bytes, and
 *     decrement max_bytes. Exit the loop based on the loop conditions
 *     and return the number of bytes read to the caller.
 *
 * Parameters:
 *     devid:     Pointer to KMI config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 kmi_read_polled(INT_32 devid,
                       void *buffer,
                       INT_32 max_bytes)
{
    KMI_REGS_T *kmiregs;
    KMI_CFG_T *kmicfgptr = (KMI_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    INT_32 bytes = 0;

    if (kmicfgptr->init == TRUE)
    {
        kmiregs = kmicfgptr->regptr;

        /* Loop until receive register is empty or until max_bytes
           expires */
        while ((max_bytes > 0) &&
            ((kmiregs->mkistatus & KMI_RX_REG_FULL) != 0))
        {
            /* Read data from register into buffer */
            *data = (UNS_8) kmiregs->mkidata;
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
 * Function: KMI_write_polled
 *
 * Purpose: KMI write function for polled mode
 *
 * Processing:
 *     If the init flag for the KMI structure is FALSE, return 0 to
 *     the caller. Otherwise, loop until n_bytes equals 0. Loop until
 *     the transmit register is full insisde the n_bytes loop. Read
 *     the data from the user buffer and place it in the KMI transmit
 *     register. Increment the address of the user buffer. Increment
 *     bytes, and decrement n_bytes. Exit the loop based on the loop
 *     conditions and return the number of actually bytes written to
 *     the caller.
 *
 * Parameters:
 *     devid:   Pointer to KMI config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 kmi_write_polled(INT_32 devid,
                        void *buffer,
                        INT_32 n_bytes)
{
    KMI_REGS_T *kmiregs;
    KMI_CFG_T *kmicfgptr = (KMI_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    INT_32 bytes = 0;

    if (kmicfgptr->init == TRUE)
    {
        kmiregs = kmicfgptr->regptr;

        /* Loop until n_bytes expires to 0 */
        while (n_bytes > 0)
        {
            if ((kmiregs->mkistatus & KMI_TX_REG_EMPTY) != 0)
            {
                /* Write data from buffer into FIFO */
                kmiregs->mkidata = (UNS_32) *data;
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
 * Function: kmi_read_ring
 *
 * Purpose: KMI read function for interrupt mode (using ring buffers)
 *
 * Processing:
 *     If the init flag for the KMI structure is FALSE, return 0 to
 *     the caller. Otherwise, save the state of the receive interrupt
 *     and disable the receive interrupt. Loop until max_bytes equals
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
 *     devid:     Pointer to KMI config structure
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
INT_32 kmi_read_ring(INT_32 devid,
                     void *buffer,
                     INT_32 max_bytes)
{
    KMI_REGS_T *kmiregs;
    UNS_32 tmp1;
    KMI_CFG_T *kmicfgptr = (KMI_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    INT_32 bytes = 0;

    if (kmicfgptr->init == TRUE)
    {
        kmiregs = kmicfgptr->regptr;

        /* Temporarily lock out KMI receive interrupt during this
           read so the KMI receive interrupt won't cause problems
           with the ring buffer index values */
        tmp1 = kmiregs->mkictrl & KMI_RXINT_EN;
        kmiregs->mkictrl &= ~KMI_RXINT_EN;

        /* Loop until receive ring buffer is empty or until max_bytes
           expires */
        while ((max_bytes > 0) &&
            (kmicfgptr->rb.rx_tail != kmicfgptr->rb.rx_head))
        {
            /* Read data from ring buffer into user buffer */
            *data = kmicfgptr->rb.rx[kmicfgptr->rb.rx_tail];
            data++;

            /* Update tail pointer */
            kmicfgptr->rb.rx_tail++;

            /* Make sure tail didn't overflow */
            if (kmicfgptr->rb.rx_tail >= KMI_RING_BUFSIZE)
            {
                kmicfgptr->rb.rx_tail = 0;
            }

            /* Increment data count and decrement buffer size count */
            bytes++;
            max_bytes--;
        }

        /* Re-enable KMI receive interrupt */
        kmiregs->mkictrl |= tmp1;
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: kmi_write_ring
 *
 * Purpose: KMI write function for interrupt mode (using ring buffers)
 *
 * Processing:
 *     If the init flag for the KMI structure is FALSE, return 0 to
 *     the caller. Otherwise, disable the transmit interrupt. Loop until
 *     n_bytes equals 0 or until the transmit ring buffer is full,
 *     whichever comes first. Write the data from the user buffer to
 *     the transmit ring buffer indexed by the head pointer. Increment
 *     the user buffer pointer and head pointer. If the head pointer
 *     exceeds the buffer size, set the head pointer to 0. Increment
 *     bytes, and decrement n_bytes. Exit the loop based on the loop
 *     conditions. If the number bytes written to the ring buffer was
 *     greater then 0, call the kmi_transmit_int() fucntion.
 *     Return the number of bytes read to the caller.
 *
 * Parameters:
 *     devid:   Pointer to KMI config structure
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
INT_32 kmi_write_ring(INT_32 devid,
                      void *buffer,
                      INT_32 n_bytes)
{
    KMI_REGS_T *kmiregs;
    KMI_CFG_T *kmicfgptr = (KMI_CFG_T *) devid;
    UNS_8 *data = (UNS_8 *) buffer;
    INT_32 bytes = 0;

    if (kmicfgptr->init == TRUE)
    {
        kmiregs = kmicfgptr->regptr;

        /* Temporarily lock out KMI transmit interrupt during this
           read so the KMI transmit interrupt won't cause problems
           with the ring buffer index values */
        kmiregs->mkictrl &= ~KMI_TXINT_EN;
        
        /* Loop until transmit run buffer is full or until n_bytes
           expires */
        while ((n_bytes > 0) &&
            (kmi_get_free_tx_count(kmicfgptr) > 0))
        {
            /* Write data from buffer into ring buffer */
            kmicfgptr->rb.tx[kmicfgptr->rb.tx_head] = *data;
            data++;

            /* Increment head pointer */
            kmicfgptr->rb.tx_head++;
            if (kmicfgptr->rb.tx_head >= KMI_RING_BUFSIZE)
            {
                kmicfgptr->rb.tx_head = 0;
            }

            /* Increment data count and decrement buffer size count */
            bytes++;
            n_bytes--;
        }

        /* Now start sending the data - this will also re-enable the
           transmit interrupt */
        if (bytes > 0)
        {
            kmi_transmit_int(kmicfgptr);
        }
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: kmi_isr
 *
 * Purpose: KMI interrupt function
 *
 * Processing:
 *     If the transmit interrupt is pending in the KMI interrupt status
 *     register, call the kmi_transmit_int() function. If the receive
 *     interrupt is pending in the KMI interrupt status register, call
 *     the kmi_receive_int() function.
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
void kmi_isr(void)
{
    /* Check for transmit interrupt first, if it is pending, call the
       ring buffer transmit function */
    if ((kmicfg.regptr->mkiint & KMI_TX_INT) != 0)
    {
        kmi_transmit_int(&kmicfg);
    }

    /* Receive interrupt */
    if ((kmicfg.regptr->mkiint & KMI_RX_INT) != 0)
    {
        kmi_receive_int(&kmicfg);
    } 
}
