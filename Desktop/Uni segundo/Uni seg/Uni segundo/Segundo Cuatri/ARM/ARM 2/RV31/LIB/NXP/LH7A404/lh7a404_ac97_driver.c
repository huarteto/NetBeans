/***********************************************************************
 * $Id:: lh7a404_ac97_driver.c 14 2007-08-28 16:08:18Z kevinw          $
 *
 * Project: LH7A404 AC97 driver
 *
 * Description:
 *     This file contains driver support for the AAC module on the
 *     LH7A404 when used with an AC97 audio CODEC.
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

#include "lh7a404_ac97_driver.h"
#include "lh7a404_dma_driver.h"

/***********************************************************************
 * AC97 driver private data and types
 **********************************************************************/

/* Array matching the AC97 bits per sample enumeration to the AAC
   control register program value */
STATIC const UNS_16 ac97_bpsam[BPSAM_20 + 1] =
{
    RXTXCR_RSIZE(0x3),    /* 12-bit data */
    RXTXCR_RSIZE(0x0),    /* 16-bit data */
    RXTXCR_RSIZE(0x1),    /* 18-bit data */
    RXTXCR_RSIZE(0x2)     /* 20-bit data */
};

/* Array used to map volume control enumerations to AC97 registers */
STATIC const UNS_16 volume_reg[MONO_OUT + 1] =
{
    AC97_MASTER_VOL,     /* AC97 Register for master volume */
    AC97_AUX_OUT_VOL,    /* AC97 Register for aux volume */
    AC97_MONO_VOL        /* AC97 Register for mono volume */
};

/* Array used to map an input to a gain register */
STATIC const UNS_16 mix_reg[PCM_OUT + 1] =
{
    AC97_BEEP_VOL,       /* AC97 Register for beep gain */
    AC97_PHONE_VOL,      /* AC97 Register for phone gain */
    AC97_MIC_VOL,        /* AC97 Register for mic in gain */
    AC97_LINE_IN_VOL,    /* AC97 Register for line in gain */
    AC97_CD_VOL,         /* AC97 Register for CD out gain */
    AC97_VIDEO_VOL,      /* AC97 Register for video out gain */
    AC97_AUX_IN_VOL,     /* AC97 Register for aux in gain */
    AC97_PCM_OUT_VOL     /* AC97 Register for PCM out gain */
};

/* Array matching the AC97 bytes per second enumeration to the AC97
   CODEC program value */
STATIC const UNS_16 ac97_bpsec[BPSEC_48000 + 1] =
{
    AC97_SR_8000,        /* AC97 value for 8KHz sampling */
    AC97_SR_11025,       /* AC97 value for 11.025KHz sampling */
    AC97_SR_16000,       /* AC97 value for 15KHz sampling */
    AC97_SR_22050,       /* AC97 value for 22.05KHz sampling */
    AC97_SR_24000,       /* AC97 value for 24KHz sampling */
    AC97_SR_32000,       /* AC97 value for 32KHz sampling */
    AC97_SR_44100,       /* AC97 value for 44.1KHz sampling */
    AC97_SR_48000        /* AC97 value for 48KHz sampling */
};

/* Function prototype used for polled and interrupt driven audio
   transfers */
typedef INT_32 (*AC97_RWFUNC_T) (void *, void *, INT_32);

/* Data structure used for AC97 driver functions */
typedef struct
{
    BOOL_32 init;               /* Device initialized flag */
    AAC_REGS_T *regptr;         /* Pointer to AC97 registers */
    AC97_CHANNEL_ID_T ch_id;    /* This channel's ID */
    void *transfer_addr;        /* Address of data buffer */
    INT_32 bytes_transferred;   /* Total bytes transferred */
    INT_32 transfer_size;       /* Total bytes to transfer */
    INT_32 bytes_trip;          /* Number of bytes that will transfer
                                   before calling the callback
                                   function */
    UNS_32 chintmask;           /* Channel interrupt mask */
    AC97_CB_FUNC_T cbfunc;      /* Pointer to callback function */
    AC97_RWFUNC_T wfunc;        /* Pointer to the write function */
    AC97_RWFUNC_T rfunc;        /* Pointer to the read function */
    AC97_DIRECTION_T chandir;   /* Channel direction */
    INT_32 channels;            /* Number of channels (1 = mono) */
    AC97_SAMPLE_RATE_T bpsec;   /* Bytes per second sampling rate */
    AC97_SAMPLE_SIZE_T bpsam;   /* Bits per sample */
    BOOL_32 compact;            /* Compact data flag */
    AC97_CH_STATUS_T cstatus;   /* DMA and interrupt channel status */
    AC97_TMODE_T tmode;         /* Channel transfer mode (polled,
                                   interrupt, or DMA) */
} AC97_CFG_T;

/* AC97 driver configuration structure */
STATIC AC97_CFG_T ac97cfg[CH_4 + 1];

/***********************************************************************
 * AC97 driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: ac97_creg_read
 *
 * Purpose: Returns the read value from an AC97 CODEC register
 *
 * Processing:
 *     Write the register and the read command to the slot 1 data field
 *     of the controller. Wait for the data to be returned from the
 *     AC97 CODEC (indicated by a status of GIS_SLOT2RXVALID), or when
 *     when timeout expires with a 2000 count, whichever comes first.
 *     Return the controller slot 2 data to the caller.
 *
 * Parameters:
 *     ac97regsptr: Pointer to the AC97 controller register set
 *     reg        : AC97 register to read from
 *
 * Outputs: None
 *
 * Returns: The value read from the register
 *
 * Notes: None
 *
 **********************************************************************/
STATIC UNS_16 ac97_creg_read(AAC_REGS_T *ac97regsptr,
                             UNS_16 reg)
{
    volatile int timeout;

    /* Set bit 15 high to indicate a read operation */
    ac97regsptr->s1data = (UNS_32) (0x0080 | reg);

    /* Long delay for timeout exceeds AC97 frame delay when cached
       and at 266MHz */
    timeout = 20000;
    while (((ac97regsptr->rgis & GIS_SLOT2RXVALID) == 0) &&
        (timeout > 0))
    {
        timeout--;
    }
    
    return (UNS_16) ac97regsptr->s2data;
}

/***********************************************************************
 *
 * Function: ac97_creg_write
 *
 * Purpose: Write to a AC97 CODEC register
 *
 * Processing:
 *     Perform a dummy read of the slot 2 data to clear any old data
 *     that may still be in it. Sent the command to the AC97 CODEC.
 *     Perform a small loop to make sure the command makes it to the
 *     CODEC.
 *
 * Parameters:
 *     ac97regsptr: Pointer to the AC97 controller register set
 *     reg        : AC97 register to write to
 *     val        : data to write to the register
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Normally, the GIS_SLOT1TXCOMPLETE bit in the controller status
 *     register can be polled to determine the transmit state of the
 *     last command sent to the CODEC. Because this bit may not always
 *     be reliable, a small delay is used instead to guarantee the data
 *     will always make it there.
 *
 **********************************************************************/
STATIC void ac97_creg_write(AAC_REGS_T *ac97regsptr,
                            UNS_16 reg,
                            UNS_16 val)
{
    volatile int timeout;

    /* Perform a dummy read of the slot 2 data to clear anything that
       might be in it */
    volatile UNS_32 idata = ac97regsptr->s2data;

    /* Suppress unused variable warning with some compilers */
    (void) idata;

    /* Send the data for the register through the controller to the
       CODEC */
    ac97regsptr->s2data = (UNS_32) val;
    ac97regsptr->s1data = (UNS_32) reg;

    /* It takes a few microseconds for the data to get to the CODEC, so
       this dummy delay allows a small bit of time for the data to get
       there in the case of repeated calls to this function (which may
       cause corruption if the data has been sent yet) */
    timeout = 2000;
    while (timeout > 0)
    {
        timeout--;
    }
}

/***********************************************************************
 *
 * Function: ac97_reset
 *
 * Purpose: Reset the AC97 CODEC
 *
 * Processing:
 *     If softreset is TRUE, enable an AC97 CODEC software reset by
 *     sending the reset command to the CODEC. Otherwise, perform a
 *     timed reset of the AC97 device. Wait for the CODEC to go back
 *     to the ready state. Enable the CODEC for variable rate audio
 *     and a variable rate MIC input.
 *
 * Parameters:
 *     ac97regsptr: Pointer to the AC97 controller register set
 *     softreset  : TRUE for a software reset, FALSE for hardware reset
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *    Some CODECs will ignore the AC97_VRM command bit, but this driver
 *    requires use of the AC97_VRA bit.
 *
 **********************************************************************/
STATIC void ac97_reset(AAC_REGS_T *ac97regsptr,
                       BOOL_32 softreset)
{
    BOOL_32 ready = FALSE;
    volatile INT_32 timeout = 50000;

    if (softreset == TRUE)
    {
        /* Software reset */
        ac97_creg_write(ac97regsptr, AC97_RESET, 0x0000);
    }
    else
    {
        /* Timed reset (cold reset) */
        ac97regsptr->reset = (RS_TIMEDRESET | RS_FORCEDRESET);
    }

    /* Wait for CODEC to go ready - it should not take *near* 50000
       iterations */
    while ((ready == FALSE) && (timeout > 0))
    {
        if ((timeout & 0x000000FF) == 0)
        {
            /* Limits AC97 read speed */
            if ((ac97regsptr->rgis & GIS_CODECREADY) == 0)
            {
                ready = TRUE;
            }
        }

        timeout--;
    }

    /* Set variable rate audio and MIC flags for variable sampling
       rates (by default - some CODECs may not support this, and they
       will only work at 48KHz sample rate) */
    ac97_creg_write(ac97regsptr, AC97_EXTD_STS_CTL,
        (AC97_VRA | AC97_VRM));
}

/***********************************************************************
 *
 * Function: ac97_cint_disable
 *
 * Purpose: Disable a channel interrupt
 *
 * Processing:
 *     For the selected channel, disable the interrupts indicated by
 *     the passed flag value by masking off the bits in the channel
 *     interrupt enable register. Also save the states in the driver
 *     data structure.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     flags     : Must be an 'OR'ed combination of ISR_TIS, ISR_RCIS,
 *                 ISR_RIS, and ISR_TCIS
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_cint_disable(AC97_CFG_T *ac97cfgptr,
                       UNS_32 flags)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;

    ac97regs->fifo[ac97cfgptr->ch_id].ie &= ~flags;
    ac97cfgptr->chintmask &= ~flags;
}

/***********************************************************************
 *
 * Function: ac97_cint_enable
 *
 * Purpose: Enable a channel interrupt
 *
 * Processing:
 *     For the selected channel, enable the interrupts indicated by
 *     the passed flag value by 'OR'ing in the bits in the channel
 *     interrupt enable register. Also save the states in the driver
 *     data structure.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     flags     : Must be an 'OR'ed combination of ISR_TIS, ISR_RCIS,
 *                 ISR_RIS, and ISR_TCIS
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_cint_enable(AC97_CFG_T *ac97cfgptr,
                      UNS_32 flags)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;

    ac97cfgptr->chintmask |= flags;
    ac97regs->fifo[ac97cfgptr->ch_id].ie |= flags;
}

/***********************************************************************
 *
 * Function: ac97_write_polled_16
 *
 * Purpose: AC97 write function for polled mode with 12/16 bit data
 *
 * Processing:
 *     If the transmit FIFO is empty, write the 16-bit value from the
 *     data buffer into the FIFO. Increment the data buffer to the next
 *     address. Increment bytes by 2. Loop until bytes is equal to
 *     n_bytes.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Pointer to data buffer to transfer
 *     n_bytes   : Number of bytes to transmit
 *
 * Outputs: None
 *
 * Returns: The number of bytes written to the FIFO
 *
 * Notes: None
 *
 **********************************************************************/
STATIC INT_32 ac97_write_polled_16(AC97_CFG_T *ac97cfgptr,
                                   void *buffer,
                                   INT_32 n_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_16 *data = (UNS_16 *) buffer;
    INT_32 bytes = 0;

    /* Loop until n_bytes expires to 0 */
    while (bytes < n_bytes)
    {
        if ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_TXFF) == 0)
        {
            /* Write data from buffer into FIFO */
            ac97regs->fifo[ac97cfgptr->ch_id].dr = (UNS_32) *data;
            data++;

            /* Increment data count and decrement buffer size count */
            bytes = bytes + 2;
        }
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: ac97_read_polled_16
 *
 * Purpose: AC97 read function for polled mode with 12/16 bit data
 *
 * Processing:
 *     If the receive FIFO is not empty, read the 16-bit value from the
 *     FIFO into the data buffer. Increment the data buffer to the next
 *     address. Increment bytes by 2. Loop until bytes is equal to
 *     max_bytes.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Pointer to data buffer to transfer
 *     max_bytes : Number of bytes to receive
 *
 * Outputs: None
 *
 * Returns: The number of bytes actually received
 *
 * Notes: None
 *
 **********************************************************************/
STATIC INT_32 ac97_read_polled_16(AC97_CFG_T *ac97cfgptr,
                                  void *buffer,
                                  INT_32 max_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_16 *data = (UNS_16 *) buffer;
    INT_32 bytes = 0;

    /* Loop until receive FIFO is empty or until max_bytes expires */
    while ((bytes < max_bytes) &&
        ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_RXFE) == 0))
    {
        /* Read data from FIFO into buffer */
        *data = (UNS_16) ac97regs->fifo[ac97cfgptr->ch_id].dr;
        data++;

        /* Increment data count and decrement buffer size count */
        bytes = bytes + 2;
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: ac97_write_polled_32
 *
 * Purpose: AC97 write function for polled mode with 18/20 bit data
 *
 * Processing:
 *     If the transmit FIFO is empty, write the 32-bit value from the
 *     data buffer into the FIFO. Increment the data buffer to the next
 *     address. Increment bytes by 4. Loop until bytes is equal to
 *     n_bytes.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Pointer to data buffer to transfer
 *     n_bytes   : Number of bytes to transmit
 *
 * Outputs: None
 *
 * Returns: The number of bytes written to the FIFO
 *
 * Notes: None
 *
 **********************************************************************/
STATIC INT_32 ac97_write_polled_32(AC97_CFG_T *ac97cfgptr,
                                   void *buffer,
                                   INT_32 n_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_32 *data = (UNS_32 *) buffer;
    INT_32 bytes = 0;

    /* Loop until n_bytes expires to 0 */
    while (bytes < n_bytes)
    {
        if ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_TXFF) == 0)
        {
            /* Write data from buffer into FIFO */
            ac97regs->fifo[ac97cfgptr->ch_id].dr = *data;
            data++;

            /* Increment data count and decrement buffer size count */
            bytes = bytes + 4;
        }
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: ac97_read_polled_32
 *
 * Purpose: AC97 read function for polled mode with 18/20 bit data
 *
 * Processing:
 *     If the receive FIFO is not empty, read the 32-bit value from the
 *     FIFO into the data buffer. Increment the data buffer to the next
 *     address. Increment bytes by 4. Loop until bytes is equal to
 *     max_bytes.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Pointer to data buffer to transfer
 *     max_bytes : Number of bytes to receive
 *
 * Outputs: None
 *
 * Returns: The number of bytes actually received
 *
 * Notes: None
 *
 **********************************************************************/
STATIC INT_32 ac97_read_polled_32(AC97_CFG_T *ac97cfgptr,
                                  void *buffer,
                                  INT_32 max_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_32 *data = (UNS_32 *) buffer;
    INT_32 bytes = 0;

    /* Loop until receive FIFO is empty or until max_bytes expires */
    while ((bytes < max_bytes) &&
        ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_RXFE) == 0))
    {
        /* Read data from FIFO into buffer */
        *data = ac97regs->fifo[ac97cfgptr->ch_id].dr;
        data++;

        /* Increment data count and decrement buffer size count */
        bytes = bytes + 4;
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: ac97_write_polled_compact
 *
 * Purpose: AC97 write function for polled mode with compact data
 *
 * Processing:
 *     If the transmit interrupt is not pending, write the 32-bit value
 *     from the data buffer into the FIFO. Increment the data buffer to
 *     the next address. Increment bytes by 4. Loop until bytes is equal
 *     to n_bytes.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Pointer to data buffer to transfer
 *     n_bytes   : Number of bytes to transmit
 *
 * Outputs: None
 *
 * Returns: The number of bytes written to the FIFO
 *
 * Notes: None
 *
 **********************************************************************/
STATIC INT_32 ac97_write_polled_compact(AC97_CFG_T *ac97cfgptr,
                                        void *buffer,
                                        INT_32 n_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_32 *data = (UNS_32 *) buffer;
    INT_32 bytes = 0;

    /* Loop until n_bytes expires to 0 */
    while (bytes < n_bytes)
    {
        if ((ac97regs->fifo[ac97cfgptr->ch_id].risr & ISR_TIS) != 0)
        {
            /* Read data from FIFO into buffer */
            ac97regs->fifo[ac97cfgptr->ch_id].dr = *data;
            data++;

            /* Increment data count and decrement buffer size count */
            bytes = bytes + 4;
        }
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: ac97_read_polled_compact
 *
 * Purpose: AC97 read function for polled mode with compact data
 *
 * Processing:
 *     If the receive interrupt is pending, read the 32-bit value from
 *     the FIFO into the data buffer. Increment the data buffer to the
 *     next address. Increment bytes by 4. Loop until bytes is equal to
 *     max_bytes.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Pointer to data buffer to transfer
 *     max_bytes : Number of bytes to receive
 *
 * Outputs: None
 *
 * Returns: The number of bytes actually received
 *
 * Notes:
 *     This function provides a workaround for the AC97 controller
 *     compact data mode FIFO errata. It uses the raw interrupt status
 *     to monitor the state of the FIFO instead of the status register.
 *
 **********************************************************************/
STATIC INT_32 ac97_read_polled_compact(AC97_CFG_T *ac97cfgptr,
                                       void *buffer,
                                       INT_32 max_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_32 *data = (UNS_32 *) buffer;
    INT_32 bytes = 0;

    /* Loop until receive FIFO is empty or until max_bytes expires */
    while ((bytes < max_bytes) &&
        ((ac97regs->fifo[ac97cfgptr->ch_id].risr & ISR_RIS) != 0))
    {
        /* Read data from FIFO into buffer */
        *data = ac97regs->fifo[ac97cfgptr->ch_id].dr;
        data++;

        /* Increment data count and decrement buffer size count */
        bytes = bytes + 4;
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: ac97_write_int_16
 *
 * Purpose: AC97 CODEC transmit interrupt (playback, 16-bit)
 *
 * Processing:
 *     While the transmit FIFO is not full and the number of bytes
 *     written to the FIFO has not exceeded the number of bytes to
 *     transfer, read a data value from the transfer buffer and place
 *     into the FIFO. Update the data buffer pointer and byte count.
 *     When the FIFO is full or the number of bytes has reached the
 *     transfer size, check the number of bytes transferred against
 *     the byte trip count. If it equals or exceed the trip count and
 *     the pointer to the user defined callback function is not NULL,
 *     the jump to the user defined callback function (with the number
 *     of bytes transferred so far as an argument). If the number of
 *     bytes transferred equals or exceeds the transfer count limit,
 *     disable the transmit interrupts for the channel and set the
 *     channel status to idle.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Not used
 *     n_bytes   : Not used
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_write_int_16(AC97_CFG_T *ac97cfgptr,
                              void *buffer,
                              INT_32 n_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_16 *data = (UNS_16 *) ac97cfgptr->transfer_addr;

    /* Loop until all data has been transferred on until the FIFO is
       full */
    while (((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_TXFF) == 0) &&
        (ac97cfgptr->bytes_transferred < ac97cfgptr->transfer_size))
    {
        /* Put data in FIFO */
        ac97regs->fifo[ac97cfgptr->ch_id].dr = (UNS_32) *data;

        /* Next data fetch address */
        data++;

        /* Update bytes transferred counter */
        ac97cfgptr->bytes_transferred =
            ac97cfgptr->bytes_transferred + 2;
    }

    /* Update transfer address */
    ac97cfgptr->transfer_addr = data;

    /* Has the trip point been met or exceeded? */
    if (ac97cfgptr->bytes_transferred >=
        ac97cfgptr->bytes_trip)
    {
        /* Yes, call user function with the amount of bytes that
           has been transferred so far */
        if (ac97cfgptr->cbfunc != (AC97_CB_FUNC_T) NULL)
        {
            ac97cfgptr->cbfunc(ac97cfgptr->bytes_transferred);
        }
    }

    /* Have all bytes been sent or received? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->transfer_size)
    {
        /* Yes, this channel needs to have it's interrupts disabled
           and will into idle mode, as it is complete */

        /* No more data, so disable interrupt */
        ac97_cint_disable(ac97cfgptr, ISR_TIS);

        /* Update channel status to idle */
        ac97cfgptr->cstatus = CH_IDLE;
    }
}

/***********************************************************************
 *
 * Function: ac97_read_int_16
 *
 * Purpose: AC97 CODEC receive interrupt (record, 16-bit)
 *
 * Processing:
 *     While the receive FIFO is not empty and the number of bytes
 *     read from the FIFO has not exceeded the number of bytes to
 *     transfer, read a data value from the FIFO and place it into
 *     the transfer buffer. Update the data buffer pointer and byte
 *     count. When the FIFO is empty or the number of bytes has reached
 *     the transfer size, check the number of bytes transferred against
 *     the byte trip count. If it equals or exceed the trip count and
 *     the pointer to the user defined callback function is not NULL,
 *     the jump to the user defined callback function (with the number
 *     of bytes transferred so far as an argument). If the number of
 *     bytes transferred equals or exceeds the transfer count limit,
 *     disable the receive interrupts for the channel and set the
 *     channel status to idle.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Not used
 *     max_bytes : Not used
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_read_int_16(AC97_CFG_T *ac97cfgptr,
                             void *buffer,
                             INT_32 max_bytes)
{
    UNS_16 *data = (UNS_16 *) ac97cfgptr->transfer_addr;
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;

    /* Fill the FIFO with the data from the source pointer until all
       the data has been transferred or until the FIFO is full */
    while ((ac97cfgptr->bytes_transferred < ac97cfgptr->transfer_size)
        && ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_RXFE) == 0))
    {
        /* Read data from FIFO into buffer */
        *data = (UNS_16) ac97regs->fifo[ac97cfgptr->ch_id].dr;

        /* Update pointer and bytes count */
        data++;
        ac97cfgptr->bytes_transferred =
            ac97cfgptr->bytes_transferred + 2;
    }

    /* Update transfer address for next interrupt */
    ac97cfgptr->transfer_addr = data;

    /* Has the trip point been met or exceeded? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->bytes_trip)
    {
        /* Yes, call user function with the amount of bytes that
           has been transferred so far */
        if (ac97cfgptr->cbfunc != (AC97_CB_FUNC_T) NULL)
        {
            ac97cfgptr->cbfunc(ac97cfgptr->bytes_transferred);
        }
    }

    /* Have all bytes been sent or received? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->transfer_size)
    {
        /* Yes, this channel needs to have it's interrupts disabled
           and will into idle mode, as it is complete */

        /* No more data, so disable interrupt */
        ac97_cint_disable(ac97cfgptr, (ISR_RIS | ISR_RCIS));

        /* Update channel status to idle */
        ac97cfgptr->cstatus = CH_IDLE;
    }
}

/***********************************************************************
 *
 * Function: ac97_write_int_32
 *
 * Purpose: AC97 CODEC transmit interrupt (playback, 32-bit)
 *
 * Processing:
 *     See the ac97_write_int_16 function. This function performs the
 *     same function as the ac97_write_int_16 except the data size is
 *     32-bits instead of 16-bits.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Not used
 *     n_bytes   : Not used
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_write_int_32(AC97_CFG_T *ac97cfgptr,
                              void *buffer,
                              INT_32 n_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_32 *data = (UNS_32 *) ac97cfgptr->transfer_addr;

    /* Loop until all data has been transferred on until the FIFO is
       full */
    while (((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_TXFF) == 0) &&
        (ac97cfgptr->bytes_transferred < ac97cfgptr->transfer_size))
    {
        /* Put data in FIFO */
        ac97regs->fifo[ac97cfgptr->ch_id].dr = *data;

        /* Next data fetch address */
        data++;

        /* Update bytes transferred counter */
        ac97cfgptr->bytes_transferred =
            ac97cfgptr->bytes_transferred + 4;
    }

    /* Update transfer address */
    ac97cfgptr->transfer_addr = data;

    /* Has the trip point been met or exceeded? */
    if (ac97cfgptr->bytes_transferred >=
        ac97cfgptr->bytes_trip)
    {
        /* Yes, call user function with the amount of bytes that
           has been transferred so far */
        if (ac97cfgptr->cbfunc != (AC97_CB_FUNC_T) NULL)
        {
            ac97cfgptr->cbfunc(ac97cfgptr->bytes_transferred);
        }
    }

    /* Have all bytes been sent or received? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->transfer_size)
    {
        /* Yes, this channel needs to have it's interrupts disabled
           and will into idle mode, as it is complete */

        /* No more data, so disable interrupt */
        ac97_cint_disable(ac97cfgptr, ISR_TIS);

        /* Update channel status to idle */
        ac97cfgptr->cstatus = CH_IDLE;
    }
}

/***********************************************************************
 *
 * Function: ac97_read_int_32
 *
 * Purpose: AC97 CODEC receive interrupt (record, 32-bit)
 *
 * Processing:
 *     See the ac97_read_int_16 function. This function performs the
 *     same function as the ac97_read_int_16 except the data size is
 *     32-bits instead of 16-bits.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Not used
 *     max_bytes : Not used
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_read_int_32(AC97_CFG_T *ac97cfgptr,
                             void *buffer,
                             INT_32 max_bytes)
{
    UNS_32 *data = (UNS_32 *) ac97cfgptr->transfer_addr;
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;

    /* Fill the FIFO with the data from the source pointer until all
       the data has been transferred or until the FIFO is full */
    while ((ac97cfgptr->bytes_transferred < ac97cfgptr->transfer_size)
        && ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_RXFE) == 0))
    {
        /* Read data from FIFO into buffer */
        *data = ac97regs->fifo[ac97cfgptr->ch_id].dr;

        /* Update pointer and bytes count */
        data++;
        ac97cfgptr->bytes_transferred =
            ac97cfgptr->bytes_transferred + 4;
    }

    /* Update transfer address for next interrupt */
    ac97cfgptr->transfer_addr = data;

    /* Has the trip point been met or exceeded? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->bytes_trip)
    {
        /* Yes, call user function with the amount of bytes that
           has been transferred so far */
        if (ac97cfgptr->cbfunc != (AC97_CB_FUNC_T) NULL)
        {
            ac97cfgptr->cbfunc(ac97cfgptr->bytes_transferred);
        }
    }

    /* Have all bytes been sent or received? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->transfer_size)
    {
        /* Yes, this channel needs to have it's interrupts disabled
           and will into idle mode, as it is complete */

        /* No more data, so disable interrupt */
        ac97_cint_disable(ac97cfgptr, (ISR_RIS | ISR_RCIS));

        /* Update channel status to idle */
        ac97cfgptr->cstatus = CH_IDLE;
    }
}

/***********************************************************************
 *
 * Function: ac97_write_int_compact
 *
 * Purpose: AC97 CODEC transmit interrupt (compact mode)
 *
 * Processing:
 *     See the ac97_write_int_32 function. This function performs the
 *     same function as the ac97_write_int_32 except the interrupt
 *     status bits are used instead of the FIFO status bits to determine
 *     the amount of data to move.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Not used
 *     n_bytes   : Not used
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_write_int_compact(AC97_CFG_T *ac97cfgptr,
                                   void *buffer,
                                   INT_32 n_bytes)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    UNS_32 *data = (UNS_32 *) ac97cfgptr->transfer_addr;

    /* Loop until all data has been transferred on until the FIFO is
       full */
    while (((ac97regs->fifo[ac97cfgptr->ch_id].risr & ISR_TIS) != 0) &&
        (ac97cfgptr->bytes_transferred < ac97cfgptr->transfer_size))
    {
        /* Put data in FIFO */
        ac97regs->fifo[ac97cfgptr->ch_id].dr = *data;

        /* Next data fetch address */
        data++;

        /* Update bytes transferred counter */
        ac97cfgptr->bytes_transferred =
            ac97cfgptr->bytes_transferred + 4;
    }

    /* Update transfer address */
    ac97cfgptr->transfer_addr = data;

    /* Has the trip point been met or exceeded? */
    if (ac97cfgptr->bytes_transferred >=
        ac97cfgptr->bytes_trip)
    {
        /* Yes, call user function with the amount of bytes that
           has been transferred so far */
        if (ac97cfgptr->cbfunc != (AC97_CB_FUNC_T) NULL)
        {
            ac97cfgptr->cbfunc(ac97cfgptr->bytes_transferred);
        }
    }

    /* Have all bytes been sent or received? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->transfer_size)
    {
        /* Yes, this channel needs to have it's interrupts disabled
           and will into idle mode, as it is complete */

        /* No more data, so disable interrupt */
        ac97_cint_disable(ac97cfgptr, ISR_TIS);

        /* Update channel status to idle */
        ac97cfgptr->cstatus = CH_IDLE;
    }
}

/***********************************************************************
 *
 * Function: ac97_read_int_compact
 *
 * Purpose: AC97 CODEC receive interrupt (compact mode)
 *
 * Processing:
 *     See the ac97_read_int_32 function. This function performs the
 *     same function as the ac97_read_int_32 except the interrupt
 *     status bits are used instead of the FIFO status bits to determine
 *     the amount of data to move.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Not used
 *     max_bytes : Not used
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_read_int_compact(AC97_CFG_T *ac97cfgptr,
                                  void *buffer,
                                  INT_32 max_bytes)
{
    UNS_32 *data = (UNS_32 *) ac97cfgptr->transfer_addr;
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;

    /* Fill the FIFO with the data from the source pointer until all
       the data has been transferred or until the FIFO is full */
    while ((ac97cfgptr->bytes_transferred < ac97cfgptr->transfer_size)
        && ((ac97regs->fifo[ac97cfgptr->ch_id].risr & ISR_RIS) != 0))
    {
        /* Read data from FIFO into buffer */
        *data = ac97regs->fifo[ac97cfgptr->ch_id].dr;

        /* Update pointer and bytes count */
        data++;
        ac97cfgptr->bytes_transferred =
            ac97cfgptr->bytes_transferred + 4;
    }

    /* Update transfer address for next interrupt */
    ac97cfgptr->transfer_addr = data;

    /* Has the trip point been met or exceeded? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->bytes_trip)
    {
        /* Yes, call user function with the amount of bytes that
           has been transferred so far */
        if (ac97cfgptr->cbfunc != (AC97_CB_FUNC_T) NULL)
        {
            ac97cfgptr->cbfunc(ac97cfgptr->bytes_transferred);
        }
    }

    /* Have all bytes been sent or received? */
    if (ac97cfgptr->bytes_transferred >= ac97cfgptr->transfer_size)
    {
        /* Yes, this channel needs to have it's interrupts disabled
           and will into idle mode, as it is complete */

        /* No more data, so disable interrupt */
        ac97_cint_disable(ac97cfgptr, (ISR_RIS | ISR_RCIS));

        /* Update channel status to idle */
        ac97cfgptr->cstatus = CH_IDLE;
    }
}

/***********************************************************************
 *
 * Function: ac97_read_write_null
 *
 * Purpose: AC97 null read and write function (used with DMA only)
 *
 * Processing:
 *     This function does nothing and is only used as a read and write
 *     stub function when using DMA. (Actual DMA functions do not use
 *     the read and write functions).
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *     buffer    : Not used
 *     max_bytes : Not used
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_read_write_null(AC97_CFG_T *ac97cfgptr,
                                 void *buffer,
                                 INT_32 max_bytes)
{
    /* Do nothing */
    ;
}

/***********************************************************************
 *
 * Function: ac97_configure_ch
 *
 * Purpose: Configure an AC97 channel for playback or record
 *
 * Processing:
 *     This function configures an AC97 channel for playback or record
 *     and sets up parameters such as sample rate, sample size, data
 *     transfer modes, and functions indirectly called via ac97_read
 *     and ac97_write.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to AC97 config structure
 *     chcfg     : Pointer to a channel configuration structure
 *
 * Outputs: None
 *
 * Returns:
 *     _NO_ERROR for interrupt and polling mode, or the DMA channel
 *     for the transfer in DMA mode.
 *
 * Notes: None
 *
 **********************************************************************/
STATIC STATUS ac97_configure_ch(AC97_CFG_T *ac97cfgptr,
                              AC97_CHANNEL_CONFIG_T *chcfg)
{
    UNS_32 config;
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    STATUS status = _NO_ERROR;

    /* In polling mode? */
    if (chcfg->tmode == AC97_POLLED)
    {
        /* If the mode is not compact mode and the same size is
           12 or 16 bits, then use the standard 16-bit polled
           functions */
        if (chcfg->ssize <= BPSAM_16)
        {
            /* Make sure there are at least 2 channels - compact mode
               doesn't work in one channel (and may not work with more
               than 2 channels) */
            if (chcfg->channels < 2)
            {
                /* Not a stereo mode, disable compact mode */
                chcfg->compact = FALSE;
            }

            /* If using compact mode, use the 32-bit versions of the
               polled data functions */
            if (chcfg->compact == TRUE)
            {
                ac97cfgptr->wfunc =
                    (AC97_RWFUNC_T) ac97_write_polled_compact;
                ac97cfgptr->rfunc =
                    (AC97_RWFUNC_T) ac97_read_polled_compact;
            }
            else
            {
                /* Use the polled 12/16 bit data transfer functions as
                   the default read/write functions */
                ac97cfgptr->wfunc =
                    (AC97_RWFUNC_T) ac97_write_polled_16;
                ac97cfgptr->rfunc =
                    (AC97_RWFUNC_T) ac97_read_polled_16;
            }
        }
        else
        {
            /* Use the 32-bit versions instead */
            ac97cfgptr->wfunc =
                (AC97_RWFUNC_T) ac97_write_polled_32;
            ac97cfgptr->rfunc =
                (AC97_RWFUNC_T) ac97_read_polled_32;

            /* Compact mode not valid in 18/20 bit modes */
            chcfg->compact = FALSE;
        }
    }
    else if (chcfg->tmode == AC97_INTERRUPT)
    {
        /* If the mode is not compact mode and the same size is
           12 or 16 bits, then use the standard 16-bit interrupt
           functions */
        if (chcfg->ssize <= BPSAM_16)
        {
            /* Make sure there are at least 2 channels - compact mode
               doesn't work in one channel (and may not work with more
               than 2 channels) */
            if (chcfg->channels < 2)
            {
                /* Not a stereo mode, disable compact mode */
                chcfg->compact = FALSE;
            }

            /* If using compact mode, use the 32-bit versions of the
               interrupt data functions */
            if (chcfg->compact == TRUE)
            {
                ac97cfgptr->wfunc =
                    (AC97_RWFUNC_T) ac97_write_int_compact;
                ac97cfgptr->rfunc =
                    (AC97_RWFUNC_T) ac97_read_int_compact;
            }
            else
            {
                /* Use the polled 12/16 bit data transfer functions as
                   the default read/write functions */
                ac97cfgptr->wfunc = (AC97_RWFUNC_T) ac97_write_int_16;
                ac97cfgptr->rfunc = (AC97_RWFUNC_T) ac97_read_int_16;
            }
        }
        else
        {
            /* Use the 32-bit versions instead */
            ac97cfgptr->wfunc = (AC97_RWFUNC_T) ac97_write_int_32;
            ac97cfgptr->rfunc = (AC97_RWFUNC_T) ac97_read_int_32;

            /* Compact mode not valid in 18/20 bit modes */
            chcfg->compact = FALSE;
        }
    }
    else /* DMA driven modes */
    {
        /* For DMA, the mode must be compact mode */
        chcfg->compact = TRUE;

        /* Determine the DMA channel to return to the caller for the
           selected AC97 channel */
        if (chcfg->dir == PLAYBACK)
        {
            /* One of 3 TX DMA channels */
            switch (ac97cfgptr->ch_id)
            {
                case CH_1:
                    status = (STATUS) DMAC_AAC_TX1;
                    break;

                case CH_2:
                    status = (STATUS) DMAC_AAC_TX2;
                    break;

                case CH_3:
                    status = (STATUS) DMAC_AAC_TX3;
                    break;
            
                default:
                    status = (STATUS) DMAC_CHANNEL_NONE;
                    break;
            }
        }
        else
        {
            /* One of 3 RX DMA channels */
            switch (ac97cfgptr->ch_id)
            {
                case CH_1:
                    status = (STATUS) DMAC_AAC_RX1;
                    break;

                case CH_2:
                    status = (STATUS) DMAC_AAC_RX2;
                    break;

                case CH_3:
                    status = (STATUS) DMAC_AAC_RX3;
                    break;
            
                default:
                    status = (STATUS) DMAC_CHANNEL_NONE;
                    break;
            }
        }

        /* Set pointers to NULL read and write functions, as DMA
           doesn't really use these functions */
        ac97cfgptr->wfunc = (AC97_RWFUNC_T) ac97_read_write_null;
        ac97cfgptr->rfunc = (AC97_RWFUNC_T) ac97_read_write_null;
    }

    /* Save additional data about the audio data */
    ac97cfgptr->channels = chcfg->channels;
    ac97cfgptr->bpsam    = chcfg->ssize;
    ac97cfgptr->bpsec    = chcfg->srate;
    ac97cfgptr->compact  = chcfg->compact;

    /* Save channel mode */
    ac97cfgptr->tmode = chcfg->tmode;
    
    /* Build the AAC channel configuration word */
    config = 0;
    if (chcfg->compact == TRUE)
    {
        /* Data is compacted */
        config = RXTXCR_CM;
    }

    /* Add in bits per sample */
    config |= ac97_bpsam[chcfg->ssize];

    /* Add in channels to config word */
    switch (chcfg->channels)
    {
        case 3:
            /* Center channel */
            config |= RXTXR_RX(5);

        case 2:
            /* Right channel */
            config |= RXTXR_RX(4);

        case 1:
        default:
            /* Left channel or mono */
            config |= RXTXR_RX(3);
    }

    if (chcfg->dir == PLAYBACK)
    {
        /* Write sample output rate value to CODEC */
        ac97_creg_write(ac97regs, AC97_FR_DAC_RATE,
            ac97_bpsec[chcfg->srate]);

        /* Update AAC transmit register with new channel
           configuration */
        ac97cfgptr->chandir = PLAYBACK;
        ac97regs->fifo[ac97cfgptr->ch_id].rxcr = 0;
        ac97regs->fifo[ac97cfgptr->ch_id].txcr = 0;
        ac97regs->fifo[ac97cfgptr->ch_id].txcr = config | RXTXCR_REN;

        /* Wait until transmit FIFO is empty */
        while ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_TXFE) == 0);
    }
    else
    {
        /* Write sample input rate value to CODEC */
        ac97_creg_write(ac97regs, AC97_PCM_ADC_RATE,
            ac97_bpsec[chcfg->srate]);

        /* Update AAC receive register with new channel
           configuration */
        ac97cfgptr->chandir = RECORD;
        ac97regs->fifo[ac97cfgptr->ch_id].txcr = 0;
        ac97regs->fifo[ac97cfgptr->ch_id].rxcr = 0;
        ac97regs->fifo[ac97cfgptr->ch_id].rxcr = config | RXTXCR_REN;

        /* Clean out the FIFO */
        while ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_RXFE) == 0)
        {
            /* Read data from FIFO into buffer */
            config = ac97regs->fifo[ac97cfgptr->ch_id].dr;
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: ac97_start_int
 *
 * Purpose: Start a channel playing or recording in interrupt mode
 *
 * Processing:
 *     If the channel is a playback channel, empty the transmit FIFO
 *     for the channel, set the channel status to playing, and enable
 *     the channel transmit interrupt. Otherwise, empty the receive
 *     FIFO, set the channel status to recording, and enable the
 *     receive interrupts.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
STATIC void ac97_start_int(AC97_CFG_T *ac97cfgptr)
{
    AAC_REGS_T *ac97regs = ac97cfgptr->regptr;
    volatile UNS_32 tmp;

    /* Playback channel? */
    if (ac97cfgptr->chandir == PLAYBACK)
    {
        /* Clear the transmit FIFO */
        while ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_TXFE) == 0);

        /* Set channel status to PLAYING */
        ac97cfgptr->cstatus = CH_PLAYING;

        /* Start channel transfer by enabling transmit interrupt */
        ac97_cint_enable(ac97cfgptr, ISR_TIS);
    }
    else
    {
        /* Clear the receive FIFO */
        while ((ac97regs->fifo[ac97cfgptr->ch_id].sr & SR_RXFE) == 0)
        {
            /* Read data from FIFO into buffer */
            tmp = ac97regs->fifo[ac97cfgptr->ch_id].dr;
        }

        /* Set channel status to RECORDING */
        ac97cfgptr->cstatus = CH_RECORDING;

        /* Start channel transfer by enabling receive interrupts */
        ac97_cint_enable(ac97cfgptr, (ISR_RIS | ISR_RCIS));
    }
}

/***********************************************************************
 *
 * Function: ac97_stop_int_ch
 *
 * Purpose: Stop an interrupt driven playback or record operation
 *
 * Processing:
 *     If the channel is a playback channel, disable the channel
 *     transmit interrupt. Otherwise, disable the channel receive
 *     interrupts. Set the channel's status to CH_IDLE.
 *
 * Parameters:
 *     ac97cfgptr: Pointer to a AC97 channel device config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void ac97_stop_int_ch(AC97_CFG_T *ac97cfgptr)
{
    /* Playback channel? */
    if (ac97cfgptr->chandir == PLAYBACK)
    {
        /* Disable transmit interrupt */
        ac97_cint_disable(ac97cfgptr, ISR_TIS);
    }
    else
    {
        /* Disable receive interrupts */
        ac97_cint_disable(ac97cfgptr, (ISR_RIS | ISR_RCIS));
    }

    /* Set channel status to idle */
    ac97cfgptr->cstatus = CH_IDLE;
}

/***********************************************************************
 * AC97 driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: ac97_open
 *
 * Purpose: Open the AC97 device
 *
 * Processing:
 *     If the passed device ipbase is not valid, then return _ERROR to
 *     the caller. Otherwise, determine if any channels have already
 *     been initialized. If they haven't, initialized the entire AC97
 *     driver device structure and initialize the AC97 device. Determine
 *     the first available channel (that is not allocated). If a channel
 *     is available, setup the basic channel parameters.
 *
 * Parameters:
 *     ipbase: Pointer to a AC97 controller peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a AC97 config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ac97_open(void *ipbase, INT_32 arg)
{
    AC97_CHANNEL_ID_T ch_id;
    volatile INT_32 timeout;
    INT_32 tmp;
    AAC_REGS_T *ac97regs;

    INT_32 status = 0;

    /* Is the IP base address correct? */
    if ((AAC_REGS_T *) ipbase == AAC)
    {
        ac97regs = (AAC_REGS_T *) ipbase;

        /* Is this the first time this function is called? */
        ch_id = CH_1;
        tmp = 0;
        while (ch_id <= CH_4)
        {
            if (ac97cfg[ch_id].init == TRUE)
            {
                /* Previous initialization was performed */
                tmp++;
            }

            ch_id++;
        }

        /* First time initialization of all channels */
        if (tmp == 0)
        {
            /* Default state of AC97 is enabled */
            ac97regs->gcr = GCR_AACIFE;

            /* Make sure each CODEC inteface is disabled */
            for (ch_id = CH_1; ch_id <= CH_4; ch_id++)
            {
                ac97regs->fifo[ch_id].rxcr    = 0x00000000;
                ac97regs->fifo[ch_id].txcr    = 0x00000000;
                ac97regs->fifo[ch_id].ie      = 0x00000000;
                ac97cfg[ch_id].cbfunc         = NULL;
                ac97cfg[ch_id].cstatus        = CH_FREE;
                ac97cfg[ch_id].chintmask      = 0x00000000;
            }

            /* Wait for CODEC to go ready */
            timeout = 2000;
            while (((ac97regs->rgis & GIS_CODECREADY) == 0) &&
                (timeout > 0))
            {
                timeout--;
            }
        }

        /* Find a free channel */
        ch_id = CH_1;
        while (ac97cfg[ch_id].init == TRUE)
        {
            /* Channel is allocated, next channel */
            ch_id++;
        }

        /* If all channels are allocated, return NULL, otherwise
           keep going */
        if (ch_id <= CH_4)
        {
            /* Set channel as allocated and idle */
            ac97cfg[ch_id].init = TRUE;
            ac97cfg[ch_id].cstatus = CH_IDLE;

            /* Save address of peripheral block */
            ac97cfg[ch_id].regptr = (AAC_REGS_T *) ipbase;

            /* Use the polled 12/16 bit data transfer functions as the
               default read/write functions */
            ac97cfg[ch_id].wfunc =
                (AC97_RWFUNC_T) ac97_write_polled_16;
            ac97cfg[ch_id].rfunc =
                (AC97_RWFUNC_T) ac97_read_polled_16;

            /* Save channel's ID in the structure */
            ac97cfg[ch_id].ch_id = ch_id;

            /* Return pointer to AC97 controller configuration
               structure for the selected channel */
            status = (INT_32) &ac97cfg[ch_id];
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: ac97_close
 *
 * Purpose: Close the AC97 device
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, determine if any
 *     other AC97 channels are being used. If they aren't, disable the
 *     the AC97 controller. Disable the FIFOs for the closed channel,
 *     set channel status to free, clear the init flag, and return
 *     _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to AC97 config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS ac97_close(INT_32 devid)
{
    AC97_CHANNEL_ID_T ch_id;
    INT_32 tmp;
    AC97_CFG_T *ac97cfgptr = (AC97_CFG_T *) devid;
    STATUS status = _ERROR;

    if (ac97cfgptr->init == TRUE)
    {
        /* Are any other channels initialized? */
        ch_id = CH_1;
        tmp = 0;
        while (ch_id <= CH_4)
        {
            if (ac97cfg[ch_id].init == TRUE)
            {
                /* Previous initialization was performed */
                tmp++;
            }

            ch_id++;
        }

        if (tmp <= 1)
        {
            /* This is the last enabled channel, so shutdown the AC97
               controller */
            ac97cfgptr->regptr->gcr = 0;
        }

        /* Successful operation */
        status = _NO_ERROR;

        /* Disable CODEC interface for this channel */
        ac97cfgptr->regptr->fifo[ch_id].rxcr = 0x0;
        ac97cfgptr->regptr->fifo[ch_id].txcr = 0x0;

        /* Set channel as free and clear init flag */
        ac97cfgptr->cstatus = CH_FREE;
        ac97cfgptr->init = FALSE;
    }

    return status;
}

/***********************************************************************
 *
 * Function: ac97_ioctl
 *
 * Purpose: AC97 configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to AC97 config structure
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
STATUS ac97_ioctl(INT_32 devid,
                  INT_32 cmd,
                  INT_32 arg)
{
    AAC_REGS_T *ac97regs;
    UNS_16 tmp;
    AC97_CFG_T *ac97cfgptr = (AC97_CFG_T *) devid;
    STATUS status = _ERROR;

    if (ac97cfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        ac97regs = ac97cfgptr->regptr;

        switch (cmd)
        {
            case AC97_ENABLE:
                /* Enable or disable AC97 controller */
                if (arg == 1)
                {
                    /* Enable AC97 controller */
                    ac97regs->gcr |= GCR_AACIFE;
                }
                else
                {
                    /* Disable AC97 controller */
                    ac97regs->gcr &= ~GCR_AACIFE;
                }
                break;

            case AC97_DO_RESET:
                /* Perform an AC97 reset */
                if (arg == 1)
                {
                    /* Software reset */
                    ac97_reset(ac97regs, TRUE);
                }
                else
                {
                    /* Timed reset (cold reset) */
                    ac97_reset(ac97regs, FALSE);
                }
                break;

            case AC97_SET_VOL:
                /* Sets the master volume, aux volume, or mono output
                   volume */
                if (arg != 0)
                {
                    UNS_16 volume;
                    AC97_VOLUME_T *ac97vol = (AC97_VOLUME_T *) arg;

                    if (ac97vol->mute == TRUE)
                    {
                        /* Mute volume */
                        volume = (AC97_MUTE << 8) | AC97_MUTE;
                    }
                    else
                    {
                        /* Normal volume */
                        volume = (((UNS_16) ac97vol->left) << 8) |
                            (UNS_16) ac97vol->right;
                    }

                    ac97_creg_write(ac97regs,
                        volume_reg[ac97vol->select], volume);
                }
                break;

            case AC97_SET_MIXGAIN:
                /* Set a mixer gain */
                if (arg != 0)
                {
                    UNS_16 gain;
                    AC97_MIX_GAIN_T *ac97gain = (AC97_MIX_GAIN_T *) arg;

                    if (ac97gain->mute == TRUE)
                    {
                        /* Mute volume */
                        gain = (AC97_MIX_GN_MUTE << 8) |
                            AC97_MIX_GN_MUTE;
                    }
                    else
                    {
                        /* Normal volume */
                        gain = (((UNS_16) ac97gain->left) << 8) |
                            (UNS_16) ac97gain->right;
                    }

                    ac97_creg_write(ac97regs, mix_reg[ac97gain->select],
                        gain);
                }
                break;

            case AC97_SET_TONE:
                /* Set bass and treble (tone) gains */
                if (arg != 0)
                {
                    UNS_16 tone;
                    AC97_TONE_T *ac97tone = (AC97_TONE_T *) arg;
                    tone = (((UNS_16) ac97tone->bass_gain) << 8) |
                        (UNS_16) ac97tone->treble_gain;
                    ac97_creg_write(ac97regs, AC97_MASTER_TONE, tone);
                }
                break;

            case AC97_SET_RCHAN:
                /* Set record channel */
                tmp = (((UNS_16) arg) << 8) | (UNS_16) arg;
                ac97_creg_write(ac97regs, AC97_RECORD_SEL, tmp);
                break;

            case AC97_SET_RECGAIN:
                /* Set a recording channel gain */
                if (arg != 0)
                {
                    AC97_REC_GAIN_T *recgain =
                        (AC97_REC_GAIN_T *) arg;

                    tmp = (((UNS_16) recgain->left_gain) << 8) |
                        (UNS_16) recgain->right_gain;
                    ac97_creg_write(ac97regs, AC97_RECORD_GAIN, tmp);
                }
                break;

            case AC97_SET_CHANNEL:
                /* Set channel configuration */
                if (arg != 0)
                {
                    status = ac97_configure_ch(ac97cfgptr,
                        (AC97_CHANNEL_CONFIG_T *) arg);
                }
                break;

            case AC97_INT_START:
                /* Set channel configuration */
                if (arg != 0)
                {
                    AC97_BKG_T *tmp = (AC97_BKG_T *) arg;

                    /* Setup background transfer structure */
                    ac97cfgptr->transfer_addr = tmp->buffer;
                    ac97cfgptr->transfer_size = tmp->transfer_size;
                    ac97cfgptr->bytes_trip = tmp->transfer_trip_size;
                    ac97cfgptr->cbfunc = tmp->funcptr;
                    ac97cfgptr->bytes_transferred = 0;

                    /* Start the transfer */
                    ac97_start_int(ac97cfgptr);
                }
                break;

            case AC97_STOP:
                /* Stop an interrupt or DMA driven playback or record
                   session */
                if (ac97cfgptr->tmode == AC97_INTERRUPT)
                {
                    /* Stop interrupt driven channel */
                    ac97_stop_int_ch(ac97cfgptr);

                    /* Status will return number of bytes transferred
                       to the caller */
                    status = ac97cfgptr->bytes_transferred;
                }
                break;

            case AC97_CODEC_READ:
                /* Directly reads a CODEC register */
                status = (UNS_16) ac97_creg_read(ac97regs,
                    (UNS_16) arg);
                break;

            case AC97_CODEC_WRITE:
                /* Directly writes to a CODEC register */
                ac97_creg_write(ac97regs,
                    (UNS_16) (arg & 0xFFFF),
                    (UNS_16) (arg >> 16));
                break;
                    
            case AC97_GET_STATUS:
                /* Return a AC97 controller/device status */
                switch (arg)
                {
                    case AC97_ENABLE_ST:
                        /* Return AC97 controller enabled status */
                        if ((ac97regs->gcr & GCR_AACIFE) != 0)
                        {
                            /* AC97 controller is enabled */
                            status = 1;
                        }
                        else
                        {
                            /* AC97 controller is disabled */
                            status = 0;
                        }
                        break;

                    case AC97_CHANNEL_ST:
                        /* Returns a channel status */
                        status = ac97cfgptr->cstatus;
                        break;
                        
                    case AC97_TRANSFER_CT:
                        /* Returns the number of bytes transferred */
                        status = ac97cfgptr->bytes_transferred;
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
 * Function: ac97_write
 *
 * Purpose: AC97 write function
 *
 * Processing:
 *     If the channel has been initialized and the channel has been
 *     configured as a record channel, then call the receive function
 *     configured for the channel and return it's value to the caller.
 *
 * Parameters:
 *     devid  : Pointer to AC97 config structure
 *     buffer : Pointer to write buffer
 *     n_bytes: Number of bytes to write to FIFO
 *
 * Outputs: None
 *
 * Returns: The number of bytes actually written
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ac97_write(INT_32 devid,
                  void *buffer,
                  INT_32 n_bytes)
{
    AC97_CFG_T *ac97cfgptr = (AC97_CFG_T *) devid;
    INT_32 bytes = 0;

    if ((ac97cfgptr->init == TRUE) &&
        (ac97cfgptr->chandir == PLAYBACK))
    {
        bytes = ac97cfgptr->wfunc(ac97cfgptr, buffer, n_bytes);
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: ac97_read
 *
 * Purpose: AC97 read function
 *
 * Processing:
 *     If the channel has been initialized and the channel has been
 *     configured as a record channel, then call the receive function
 *     configured for the channel and return it's value to the caller.
 *
 * Parameters:
 *     devid    : Pointer to AC97 config structure
 *     buffer   : Pointer to read buffer
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: The number of bytes actually read
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 ac97_read(INT_32 devid,
                 void *buffer,
                 INT_32 max_bytes)
{
    AC97_CFG_T *ac97cfgptr = (AC97_CFG_T *) devid;
    INT_32 bytes = 0;

    if ((ac97cfgptr->init == TRUE) &&
        (ac97cfgptr->chandir == RECORD))
    {
        bytes = ac97cfgptr->rfunc(ac97cfgptr, buffer, max_bytes);
    }

    return bytes;
}

/***********************************************************************
 *
 * Function: ac97_isr
 *
 * Purpose: Main AC97 interrupt function
 *
 * Processing:
 *     Determine the channel that the audio interrupt occurred on by
 *     checking all the channel interrupt status bits that are enabled.
 *     If the interrupt was not due to a channel (global status), then
 *     clear the global interrupts and exit. Otherwise, if the
 *     interrupt was due to a receive or receive overrun status, call
 *     the receive function for that channel. If the interrupt was due
 *     to a transmit status, call the transmit function for that
 *     channel. Otherwise, clear the global interrupts (should never
 *     happen).
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
void ac97_isr(void)
{
    AAC_REGS_T *ac97regptr;
    AC97_CHANNEL_ID_T ch_id = CH_1;

    /* Determine which AC97 interrupt and FIFO was the cause */
    while ((ch_id != CH_NONE) &&
        (((ac97cfg[ch_id].regptr->gcis >> (INT_32) ch_id) &
            ac97cfg[ch_id].chintmask) == 0))
    {
        /* Not this FIFO channel, try next channel */
        ch_id++;
    }
    
    /* If this was not a FIFO channel, then it must of been a global
       interrupt. These interrupts are not directly supported with
       this driver so the interrupt is just cleared and the function
       will return */
    if (ch_id == CH_NONE)
    {
        /* Unsupported interrupt, clear unused interrupts in
           AC97 controller */
        AAC->eoi = (EOI_WISC | EOI_CODECREADY);
    }
    else
    {
        /* Get the AC97 register pointer for the channel */
        ac97regptr = ac97cfg[ch_id].regptr;

        /* For the given fifo_id, determine the cause of the interrupt
           and perform the needed function */
        if ((ac97regptr->fifo[ch_id].isr & (ISR_RCIS | ISR_RIS)) != 0)
        {
            /* Receive interrupt */
            ac97cfg[ch_id].rfunc(&ac97cfg[ch_id], 0, 0);
        }
        else if ((ac97regptr->fifo[ch_id].isr & ISR_TIS) != 0)
        {
            /* Transmit interrupt */
            ac97cfg[ch_id].wfunc(&ac97cfg[ch_id], 0, 0);
        }
        else
        {
            /* Unsupported interrupt - this should never happen with
               this driver. If it does, semething is wrong. */
            AAC->eoi = (EOI_WISC | EOI_CODECREADY);
        }
    }
}
