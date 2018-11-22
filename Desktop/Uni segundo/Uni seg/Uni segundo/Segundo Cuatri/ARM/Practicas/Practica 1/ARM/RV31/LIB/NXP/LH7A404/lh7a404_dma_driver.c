/***********************************************************************
 * $Id:: lh7a404_dma_driver.c 14 2007-08-28 16:08:18Z kevinw           $
 *
 * Project: LH7A404 DMA driver
 *
 * Description:
 *     This file contains driver support for the DMA module on the
 *     LH7A404.
 *
 * Notes on the DMA callback interface:
 *     The DMA callback interface requires a callback function that
 *     is called from the DMA state controller code when it needs
 *     more data or when an error occurs. The callback API is as
 *     follows:
 *         void dma_cback(UNS_32 *data, UNS_32 *bytes, BOOL_32 *abort)
 *     The DMA state controller and callback function pass data
 *     between each other as follows:
 *     When the DMA state controller calls the callback function:
 *         *data contains the address of the last buffer transferred
 *         *bytes contains the number of bytes transferred in the last
 *                transfer - if the current transfer is not complete,
 *                a size of 0 is passed
 *         *abort is TRUE if an error occured - this is a signal to
 *                the callback function that a DMA error occurred and
 *                DMA is halting after this callback.
 *     When the callback functionreturns to the DMA state controller:
 *         *data contains the address of the next buffer to transfer
 *         *bytes contains the number of bytes to transfer - if no
 *                current buffers are ready to send, use a value of 0
 *                and then restart DMA later with a call to dmac_restart
 *         *abort should return TRUE to the DMA controller if this is
 *                the last transfer
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
#include "lh7a404_dma_driver.h"
#include "lh7a404_vic_driver.h"

/***********************************************************************
 * DMA driver private types
 **********************************************************************/
 
/* Maximum DMA buffer size */
#define MAX_DMA_BYTES 0xFFF0

/* DMA channel register block selection enumeration */
typedef enum
{
    DMAR_M2P_TX0 = 0,   /* DMA TX0 M2P register index */
    DMAR_M2P_RX1,       /* DMA RX1 M2P register index */
    DMAR_M2P_TX2,       /* DMA TX2 M2P register index */
    DMAR_M2P_RX3,       /* DMA RX3 M2P register index */
    DMAR_M2M_CH0,       /* DMA CH0 M2M register index */
    DMAR_M2M_CH1,       /* DMA CH1 M2M register index */
    DMAR_RSVD_1,        /* DMA reserved register index */
    DMAR_RSVD_2,        /* DMA reserved register index */
    DMAR_M2P_RX5,       /* DMA RX5 M2P register index */
    DMAR_M2P_TX4,       /* DMA TX4 M2P register index */
    DMAR_M2P_RX7,       /* DMA RX7 M2P register index */
    DMAR_M2P_TX6,       /* DMA TX6 M2P register index */
    DMAR_M2P_RX9,       /* DMA RX9 M2P register index */
    DMAR_M2P_TX9        /* DMA TX8 M2P register index */
} DMAC_CHANNEL_MAP_T;

/* DMA channel direction tag type */
typedef enum
{
    DMA_RX = 0,      /* DMA channel is receive */
    DMA_TX,          /* DMA channel is transmit */
    DMA_RXTX         /* DMA channel is receive/transmit (M2M) */
} DMAC_DIR_T;

/* DMA driver structure that maps the dma_channel_types enumeration to
   the proper DMA channel direction, CSC clock value, and interrupt
   mask */
typedef struct
{
    DMAC_DIR_T         ch_dir;  /* DMA channel direction */
    CSC_DMA_CHANNEL_T  csc_ch;  /* DMA CSC clock enable flag */
    UNS_32             dmask;   /* DMA interrupt check mask */
    VIC_INT_SOURCE_T   intid;   /* DMA VIC interrupt ID */
} DMAC_MAP_T;

/* DMA driver structure that maps the selected peripheral to the
   PALLOC value and data direction */
typedef struct
{
    UNS_32 pallocval;        /* PALLOC value for peripheral */
    DMAC_DIR_T ddir;         /* Direction for the device */
} DMAC_PDIR_T;

/* DMA transistion states for when DMA is enabled */
typedef enum
{
    DMA_STALL_STALL,         /* STALL->STALL state (used to indicate
                                a transfer request from an idle or
                                stalled channel */
    DMA_ON_STALL,            /* ON state that will drop to STALL */
    DMA_NEXT_ON              /* NEXT state will drop to ON */
} DMAC_TRAN_STATE_T;

/* DMA single channel driver control structure */
typedef struct
{
    BOOL_32 alloc;           /* Channel allocated flag */
    DMAC_CHANNEL_MAP_T ch;   /* Allocated DMA channel index */
    DMACFUNC_T cb_func;      /* Callback function */
    UNS_32     bytestr;      /* Number of bytes transferred so far */
    BOOL_32    ignore;       /* Ignore channel errors */
    BOOL_32    complete;     /* Stop at completion of transfer flag */
    UNS_8      *bfaddr;      /* Buffer address for set buffer size */
    UNS_32     bfsize;       /* Number of bytes to transfer, set buffer
                                size */
    INT_32     last_ch_ret;  /* Last channel value returned to CB */
    INT_32     packets_inxfer; /* Current packets in transfer */
    BOOL_32    next_state;   /* NEXT state flag */
} DMAC_DRV_CONTROL_T;

/***********************************************************************
 * DMA driver private data
 **********************************************************************/
 
/* This array maps the DMAC_CHANNEL_MAP_T enumeration to the proper
   channel direction and interrupt mask */
const DMAC_MAP_T dmac_map[DMAR_M2P_TX9 + 1] =
{
    {DMA_TX,   DMA_M2P_CH0, DMAC_CH0_INT,   VIC_DMA_M2PCH0},
    {DMA_RX,   DMA_M2P_CH1, DMAC_CH1_INT,   VIC_DMA_M2PCH1},
    {DMA_TX,   DMA_M2P_CH2, DMAC_CH2_INT,   VIC_DMA_M2PCH2},
    {DMA_RX,   DMA_M2P_CH3, DMAC_CH3_INT,   VIC_DMA_M2PCH3},
    {DMA_RXTX, DMA_M2M_CH0, DMAC_MMCH0_INT, VIC_DMA_M2MCH0},
    {DMA_RXTX, DMA_M2M_CH1, DMAC_MMCH1_INT, VIC_DMA_M2MCH1},
    {DMA_RXTX, DMA_M2P_CH0, 0,              (VIC_INT_SOURCE_T) 0},
    {DMA_RXTX, DMA_M2P_CH0, 0,              (VIC_INT_SOURCE_T) 0},
    {DMA_RX,   DMA_M2P_CH5, DMAC_CH5_INT,   VIC_DMA_M2PCH5},
    {DMA_TX,   DMA_M2P_CH4, DMAC_CH4_INT,   VIC_DMA_M2PCH4},
    {DMA_RX,   DMA_M2P_CH7, DMAC_CH7_INT,   VIC_DMA_M2PCH7},
    {DMA_TX,   DMA_M2P_CH6, DMAC_CH6_INT,   VIC_DMA_M2PCH6},
    {DMA_RX,   DMA_M2P_CH9, DMAC_CH9_INT,   VIC_DMA_M2PCH9},
    {DMA_TX,   DMA_M2P_CH8, DMAC_CH8_INT,   VIC_DMA_M2PCH8}
};

/* Array that maps the user selected channel type with the correct
   PALLOC value and channel direction */
const DMAC_PDIR_T dma_palloc[DMAC_UART_TX3 + 1] =
{
    {DMAC_RX_USB_SEL,   DMA_RX},
    {DMAC_TX_USB_SEL,   DMA_TX},
    {DMAC_RX_SDMMC_SEL, DMA_RX},
    {DMAC_TX_SDMMC_SEL, DMA_TX},
    {DMAC_RX_AC971_SEL, DMA_RX},
    {DMAC_TX_AC971_SEL, DMA_TX},
    {DMAC_RX_AC972_SEL, DMA_RX},
    {DMAC_TX_AC973_SEL, DMA_TX},
    {DMAC_RX_AC972_SEL, DMA_RX},
    {DMAC_TX_AC973_SEL, DMA_TX},
    {DMAC_RX_UART1_SEL, DMA_RX},
    {DMAC_TX_UART1_SEL, DMA_TX},
    {DMAC_RX_UART2_SEL, DMA_RX},
    {DMAC_TX_UART2_SEL, DMA_TX},
    {DMAC_RX_UART3_SEL, DMA_RX},
    {DMAC_TX_UART3_SEL, DMA_TX}
};

/* DMA channel driver control data */
DMAC_DRV_CONTROL_T dmac_drvctl[DMAC_UART_TX3 + 1];

/* Array used to backmap an allocated DMA channel to a peripheral */
DMAC_CHANNEL_T dma_to_periph[DMAR_M2P_TX9 + 1];

/***********************************************************************
 * DMA driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: dma_cb_tx
 *
 * Purpose: DMA data feed callback function handler
 *
 * Processing:
 *     This function handles the DMA feed and buffer return mechanism
 *     for the callback function. Determine the next buffer to use for
 *     DMA. Get the last buffer size based on the current number of
 *     transferred packets and the DMA state. Return the buffer and
 *     get a new buffer. Update buffer management based on the current
 *     DMA state and whether a new buffer was received.
 *
 * Parameters:
 *     channel: Must be an enumeration of type DMAC_CHANNEL_T
 *
 * Outputs: None
 *
 * Returns: The number of bytes to be transferred in the next DMA cycle
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 dma_cb_tx(DMAC_CHANNEL_T channel)
{
    UNS_32 buffer, buffer_size, dmastatus, dstate;
    INT_32 bfindex;
    BOOL_32 completed;
    volatile UNS_32 temp, tempcn;
    DMAC_CHANNEL_MAP_T chidx = dmac_drvctl[channel].ch;

    /* Latch DMA status */
    dmastatus = DMAC->chregs[chidx].cntl_stat;
    dstate = DMAC_CURRENTSTATE(dmastatus);

    /* Disable current DMA interrupts */
    tempcn = DMAC->chregs[chidx].control & ~DMAC_NFB;
    DMAC->chregs[chidx].control = tempcn;
    temp = DMAC->chregs[chidx].control;

    /* Determine the next buffer index for transferring data */
    if ((dmastatus & DMAC_NEXTBUFFER) == 0)
    {
        /* Use buffer 0 as the next transfer buffer */
        bfindex = 0;
    }
    else
    {
        /* Use buffer 1 as the next transfer buffer */
        bfindex = 1;
    }

    /* If no data has been previously transferred, then set
       the byte count to 0, otherwise use the transferred byte
       count for the first finished channel */
    if (dmac_drvctl[channel].packets_inxfer == 0)
    {
        buffer_size = 0;
    }
    else
    {
        if (dstate == DMAC_STATE_STALL)
        {
            /* State is stalled, so the completed packeted size is the
               last transferred channel */
            /* Clear counts */
            buffer_size = DMAC->chregs[chidx].addr_cnt[
                dmac_drvctl[channel].last_ch_ret].maxcnt;
            dmac_drvctl[channel].last_ch_ret = 1 -
                dmac_drvctl[channel].last_ch_ret;
            dmac_drvctl[channel].packets_inxfer--;
        }
        else
        {
            /* State is currently ON, if this was a transfer from the
               NEXT state, then we need to give back a packet now */
            if (dmac_drvctl[channel].next_state == TRUE)
            {
                /* NEXT->ON state transistion, give back packet */
                buffer_size = DMAC->chregs[chidx].addr_cnt[
                    dmac_drvctl[channel].last_ch_ret].maxcnt;
                dmac_drvctl[channel].last_ch_ret = 1 -
                    dmac_drvctl[channel].last_ch_ret;
                dmac_drvctl[channel].packets_inxfer--;
            }
            else
            {
                /* ON->NEXT state transistion attempt, packet size to
                   return is zero, as the packet is still being used by
                   the DMA engine */
                buffer_size = 0;
            }
        }
    }

    /* Clear NEXT state flag */
    dmac_drvctl[channel].next_state = FALSE;

    /* Call callback function with previous transfer size */
    completed = FALSE;
    dmac_drvctl[channel].cb_func(&buffer, &buffer_size, &completed);

    /* Save state of complete flag */
    dmac_drvctl[channel].complete = completed;

    /* Is there more data to transfer? */
    if (buffer_size != 0)
    {
        DMAC->chregs[chidx].addr_cnt[bfindex].maxcnt = buffer_size;
        DMAC->chregs[chidx].addr_cnt[bfindex].base = buffer;

        /* If previous state was on, then enable STALL interrupt
           only */
        if (dstate == DMAC_STATE_ON)
        {
            /* Previous state was on, enable NFB and stall interrupts */
            DMAC->chregs[chidx].control |= (tempcn | DMAC_NFB |
                DMAC_STALL);
            temp = DMAC->chregs[chidx].control;
            dmac_drvctl[channel].next_state = TRUE;
        }

        if (dmac_drvctl[channel].last_ch_ret == -1)
        {
            dmac_drvctl[channel].last_ch_ret = bfindex;
        }
        dmac_drvctl[channel].packets_inxfer++;
    }
    else
    {
        if (dstate == DMAC_STATE_STALL)
        {
            /* DMA is stalled, disable DMA stall interrupt */
            DMAC->chregs[chidx].control = (tempcn & ~DMAC_STALL);
            temp = DMAC->chregs[chidx].control;
        }
        else if (dmac_drvctl[channel].packets_inxfer == 0)
        {
            /* Enable stall interrupt */
            DMAC->chregs[chidx].control |= (tempcn | DMAC_STALL);
            temp = DMAC->chregs[chidx].control;
        }
    }    

    return buffer_size;
}

/***********************************************************************
 *
 * Function: dma_data_feed
 *
 * Purpose: DMA data feed function
 *
 * Processing:
 *     This function is called when it has been determined that a DMA
 *     channel needs data (due to a stalled or next buffer condition).
 *     If a callback function exists for the channel, call the function
 *     to get the address and size of the next buffer, and the abort
 *     flag. If a callback function doesn't exist, then the transfer is
 *     from a preallocated buffer with a set size. Compute the index
 *     into the saved buffer for the next buffer DMA address and
 *     update the bytes transferred count. If this is the last buffer
 *     to transfer, set the abort flag and enable abort in the DMA
 *     control word. Determine the next buffer index and update the
 *     transfer count and buffer address for the next DMA transfer.
 *
 * Parameters:
 *     channel: Must be an enumeration of type DMAC_CHANNEL_T
 *
 * Outputs: None
 *
 * Returns: The number of bytes transferred
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 dma_data_feed(DMAC_CHANNEL_T channel)
{
    UNS_32 buffer, buffer_size;
    INT_32 bfindex;
    DMAC_CHANNEL_MAP_T chidx;
    volatile UNS_32 temp;

    /* Does a callback function exist? */
    if (dmac_drvctl[channel].cb_func == (DMACFUNC_T) 0x0)
    {
        /* Determine the next buffer for transfer, either in the STALL
           or ON states */
        if (((DMAC->chregs[dmac_drvctl[channel].ch].cntl_stat) &
            DMAC_NEXTBUFFER) == 0)
        {
            /* Use buffer 0 as the next transfer buffer */
            bfindex = 0;
        }
        else
        {
             /* Use buffer 1 as the next transfer buffer */
            bfindex = 1;
        }

        /* Determine number of bytes to transfer in next buffer */
        buffer_size = dmac_drvctl[channel].bfsize -
            dmac_drvctl[channel].bytestr;
        if (buffer_size > MAX_DMA_BYTES)
        {
            /* Limit transfer size to DMA buffer size */
            buffer_size = MAX_DMA_BYTES;
        }

        /* Compute address to next buffer */
        buffer = (UNS_32) (dmac_drvctl[channel].bfaddr +
            dmac_drvctl[channel].bytestr);

        /* Update transfer size count */
        dmac_drvctl[channel].bytestr =
            dmac_drvctl[channel].bytestr + buffer_size;

        /* If no more bytes are left to transfer after this one,
           then abort after transfer when complete */
        if (dmac_drvctl[channel].bytestr >= dmac_drvctl[channel].bfsize)
        {
            /* This is the last transfer in the block */
            dmac_drvctl[channel].complete = TRUE;
        }

        /* Start DMA transfer */
        chidx = dmac_drvctl[channel].ch;
        if (buffer_size > 0)
        {            
            DMAC->chregs[chidx].addr_cnt[bfindex].maxcnt = buffer_size;
            DMAC->chregs[chidx].addr_cnt[bfindex].base = buffer;
            dmac_drvctl[channel].packets_inxfer = 1;
        }
        else
        {
            /* No more packets in transfer, or this is the last packet
               to transfer, so stop interrupts after this packet */
            dmac_drvctl[channel].packets_inxfer = 0;
            
            /* Disable NFB and stall interrupts */
            DMAC->chregs[chidx].control &=  ( ~(DMAC_NFB | DMAC_STALL) );
            temp = DMAC->chregs[chidx].control;
        }
    }
    else /* Transfer using the callback function */
    {
        /* Attempt to get more data and clear transferred data */
        buffer_size = dma_cb_tx(channel);
    }

    return buffer_size;
}

/***********************************************************************
 *
 * Function: dmac_state_handler
 *
 * Purpose: DMA state handler function
 *
 * Processing:
 *     This function is called to start a DMA transfer or continue an
 *     existing one. Loop until the DMA status is in NEXT state or
 *     until the byte channel transfer count is 0. In the loop, call
 *     the dma_data_feed() function.
 *
 * Parameters:
 *     channel: Must be an enumeration of type DMAC_CHANNEL_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void dmac_state_handler(DMAC_CHANNEL_T channel)
{
    INT_32 bytes = 1;

    /* Transfer until DMA is full or until no more data to send */
    while ((bytes != 0) && (DMAC_CURRENTSTATE(DMAC->chregs[
        dmac_drvctl[channel].ch].cntl_stat) != DMAC_STATE_NEXT))
    {
        /* Get data to send */
        bytes = dma_data_feed(channel);
    }
}

/***********************************************************************
 *
 * Function: dmac_start_xfer
 *
 * Purpose: Start a DMA channel transfer using a callback function
 *
 * Processing:
 *     Prior to starting the DMA transfer, call the dmac_stop function
 *     to ensure the channel is disabled. Setup the initial values of
 *     the driver control data structure used in the DMA driver state
 *     control function. Set the maxcount values for both DMA buffers
 *     to 0. Depending on the state of no_terminate, enable the DMA
 *     channel with or without channel error termination. Start the
 *     transfer process with a call to dma_data_feed().
 *
 * Parameters:
 *     channel     : Must be an enumeration of type DMAC_CHANNEL_T
 *     dcb_func    : Pointer to the DMA service callback function
 *     no_terminate: Set to TRUE to ignore channel errors
 *
 * Outputs: None
 *
 * Returns:
 *     TRUE if the transfer was started, FALSE if the channel was
 *     already being used or a channel is not available for allocation.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 dmac_start_xfer(DMAC_CHANNEL_T channel,
                        DMACFUNC_T dcb_func,
                        void *buffer,
                        UNS_32 bytes,
                        BOOL_32 no_terminate)
{
    volatile UNS_32 temp;
    DMAC_CHANNEL_MAP_T chfind;
    BOOL_32 search;
    BOOL_32 allocated = FALSE;

    /* Do not continue if the peripheral is already allocated a DMA
       channel */
    if (dmac_drvctl[channel].alloc == FALSE)
    {
        /* Find the first available unallocated DMA channel for the
           peripheral */
        search = TRUE;
        chfind = DMAR_M2P_TX0;
        while ((search == TRUE) && (chfind < DMAR_M2P_TX9))
        {
            if ((DMAC_CURRENTSTATE(
                DMAC->chregs[chfind].cntl_stat) == DMAC_STATE_IDLE) &&
                (dma_palloc[channel].ddir == dmac_map[chfind].ch_dir))
            {
                /* DMA channel is available */
                search = FALSE;
            }
            else
            {
                chfind++;
            }
        }

        /* Continue if channel is valid */
        if (chfind <= DMAR_M2P_TX9)
        {
            /* Make sure DMA interrupts are disabled */
            dmac_channel_int_enable(channel, 0);

            /* For the selected channel, turn on the DMA clock in the
               CSC */
            csc_enable_dma_clock(dmac_map[chfind].csc_ch, TRUE);

            /* Save allocated channel mapped to peripheral */
            dmac_drvctl[channel].ch = chfind;

            /* Set channel allocation status to TRUE */
            dmac_drvctl[channel].alloc = TRUE;

            /* Save backmap of DMA channel to peripheral */
            dma_to_periph[chfind] = channel;

            /* Setup the initial channel driver control data to use a
               set buffer size and location */
            dmac_drvctl[channel].cb_func = dcb_func;
            dmac_drvctl[channel].bfaddr = (UNS_8 *) buffer;
            dmac_drvctl[channel].bfsize = bytes;
            dmac_drvctl[channel].bytestr = 0;
            dmac_drvctl[channel].ignore = no_terminate;
            dmac_drvctl[channel].complete = FALSE;
            dmac_drvctl[channel].last_ch_ret = -1;
            dmac_drvctl[channel].packets_inxfer = 0;
            dmac_drvctl[channel].next_state = FALSE;

            /* Start the transfer */
            allocated = TRUE;

            /* Map the DMA channel to the peripheral in the DMA
               controller */
            DMAC->chregs[chfind].palloc = dma_palloc[channel].pallocval;

            /* Clear error interrupt status */
            DMAC->chregs[chfind].interrupt = DMAC_CHERROR;

            /* Perform an enable/disable sequence of the DMA channel,
               this will help guarantee that the byte packers are empty on
               the subsequent enable */
            DMAC->chregs[chfind].control = DMAC_ENABLE;
            temp = DMAC->chregs[chfind].control;
            DMAC->chregs[chfind].control = 0;
            temp = DMAC->chregs[chfind].control;

            /* If the no_terminate flag is TRUE, then enable the DMA
               channel with the ICE flag. This is useful for audio
               channels where occasional data errors can be ignored and
               will not terminate the DMA process with an error. */
            if (no_terminate == TRUE)
            {
                DMAC->chregs[chfind].control =
                    (DMAC_ENABLE | DMAC_CHERROR | DMAC_ICE |
                    DMAC_STALL | DMAC_NFB);
            }
            else
            {
                DMAC->chregs[chfind].control =
                    (DMAC_ENABLE | DMAC_STALL | DMAC_NFB |
                    DMAC_CHERROR);
            }
            temp = DMAC->chregs[chfind].control;

            /* Start transfer by enabling interrupts */
            dmac_state_handler(channel);
            dmac_channel_int_enable(channel, 1);
        }
    }

    return allocated;
}

/***********************************************************************
 * DMA driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: dmac_start_using_cb
 *
 * Purpose: Start a DMA channel transfer using a callback function
 *
 * Processing:
 *     Call the dmac_start_xfer with the passed parameters and a
 *     NULL buffer and size.
 *
 * Parameters:
 *     channel     : Must be an enumeration of type DMAC_CHANNEL_T
 *     dcb_func    : Pointer to the DMA service callback function
 *     no_terminate: Set to TRUE to ignore channel errors
 *
 * Outputs: None
 *
 * Returns:
 *     TRUE if the transfer was started, FALSE if the channel was
 *     already being used or a channel is not available for allocation.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 dmac_start_using_cb(DMAC_CHANNEL_T channel,
                            DMACFUNC_T dcb_func,
                            BOOL_32 no_terminate)
{
    return dmac_start_xfer(channel, dcb_func, (void *) NULL, 0,
        no_terminate);
}

/***********************************************************************
 *
 * Function: dmac_start
 *
 * Purpose: Start a DMA channel transfer using a set buffer size
 *
 * Processing:
 *     Call the dmac_start_xfer with the passed parameters and a
 *     NULL callback function.
 *
 * Parameters:
 *     channel     : Must be an enumeration of type DMAC_CHANNEL_T
 *     buffer      : Pointer to the DMA data buffer
 *     bytes       : Number of bytes to get or send
 *     no_terminate: Set to TRUE to ignore channel errors
 *
 * Outputs: None
 *
 * Returns:
 *     TRUE if the transfer was started, FALSE if the channel was
 *     already being used or a channel is not available for allocation.
 *
 * Notes:
 *     This function is the same as the dmac_start_using_cb() function
 *     except this one allows the set buffer to be used instead of a
 *     callback function.
 *
 **********************************************************************/
BOOL_32 dmac_start(DMAC_CHANNEL_T channel,
                void *buffer,
                UNS_32 bytes,
                BOOL_32 no_terminate)
{
    return dmac_start_xfer(channel, (DMACFUNC_T) NULL, buffer,
        bytes, no_terminate);
}

/***********************************************************************
 *
 * Function: dmac_restart
 *
 * Purpose: Force a stalled channel to restart
 *
 * Processing:
 *     The passed DMA channel is restarted. This function should only
 *     be called if the channel is stalled.
 *
 * Parameters:
 *     channel: Must be an enumeration of type DMAC_CHANNEL_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Do not use this function for memory to memory transfers.
 *
 **********************************************************************/
void dmac_restart(DMAC_CHANNEL_T channel)
{
    /* Temporarily disable DMA interrupt for this channel */
    dmac_channel_int_enable(channel, 0);

    /* If the current state is stalled, restart */
    if (dmac_get_status(channel) == DMA_STALL)
    {
        /* Re-start transfer */
        dmac_state_handler(channel);
    }

    /* Re-enable interrupts to allow normal processing to continue */
    dmac_channel_int_enable(channel, 1);
}

/***********************************************************************
 *
 * Function: dmac_stop
 *
 * Purpose: Stop and deallocate a DMA channel
 *
 * Processing:
 *     The passed DMA channel is disabled and returned to the DMA
 *     channel pool.
 *
 * Parameters:
 *     channel: Must be an enumeration of type DMAC_CHANNEL_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Do not use this function for memory to memory transfers.
 *
 **********************************************************************/
void dmac_stop(DMAC_CHANNEL_T channel)
{
    volatile UNS_32 temp;

    /* Disable interrupt */
    dmac_channel_int_enable(channel, 0);

    /* Disable DMA for this channel */
    DMAC->chregs[dmac_drvctl[channel].ch].control &= ~DMAC_ENABLE;
    temp = DMAC->chregs[dmac_drvctl[channel].ch].control;

    /* Set backmapping to no channel */
    dma_to_periph[dmac_drvctl[channel].ch] = DMAC_CHANNEL_NONE;
    dmac_drvctl[channel].alloc = FALSE;

    /* For the selected channel, turn off the DMA clock in the CSC */
    csc_enable_dma_clock(dmac_map[dmac_drvctl[channel].ch].csc_ch,
        FALSE);
}

/***********************************************************************
 *
 * Function: dmac_flush
 *
 * Purpose: Flush a DMA channel
 *
 * Processing:
 *     TBD
 *
 * Parameters:
 *     channel: Must be an enumeration of type DMAC_CHANNEL_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void dmac_flush(DMAC_CHANNEL_T channel)
{
    volatile UNS_32 reg, temp;

    /* Only perform this function if the DMA channel is allocated */
    if (dmac_drvctl[channel].alloc == TRUE)
    {
        /* Get current DMA control register */
        reg = DMAC->chregs[dmac_drvctl[channel].ch].control;

        /* Disable and enable DMA to flush the channel, DMA interrupts are
           left disabled after the flush */
        DMAC->chregs[dmac_drvctl[channel].ch].control = (reg & ~DMAC_ENABLE);
        temp = DMAC->chregs[dmac_drvctl[channel].ch].control;

        /* Reset DMA channel defaults */
        dmac_drvctl[channel].complete = FALSE;
        dmac_drvctl[channel].last_ch_ret = -1;
        dmac_drvctl[channel].packets_inxfer = 0;
        dmac_drvctl[channel].next_state = FALSE;

        /* Re-enable DMA channel without interrupts */
        DMAC->chregs[dmac_drvctl[channel].ch].control = (reg &
            ~(DMAC_NFB | DMAC_STALL));
        temp = DMAC->chregs[dmac_drvctl[channel].ch].control;
    }
}

/***********************************************************************
 *
 * Function: dmac_get_status
 *
 * Purpose: Return a DMA channel status
 *
 * Processing:
 *     The DMA status register for the passed channel is read and
 *     checked. IBased on the status of the DMA channel, a value of
 *     DMAC_STATUS_T is returned to the caller. If the channel is not
 *     idle and an error has occurred and errors are not being ignored,
 *     return DMA_ERROR to the caller instead.
 *
 * Parameters:
 *     channel: Must be an enumeration of type DMAC_CHANNEL_T
 *
 * Outputs: None
 *
 * Returns: An enumeration value of type DMAC_STATUS_T
 *
 * Notes: This function is safe to use for memory to memory transfers.
 *
 **********************************************************************/
DMAC_STATUS_T dmac_get_status(DMAC_CHANNEL_T channel)
{
    DMAC_STATUS_T status;
    UNS_32 dmastatus;

    /* Determine possible DMA statuses */
    dmastatus = DMAC->chregs[dmac_drvctl[channel].ch].cntl_stat;
    status = (DMAC_STATUS_T) DMAC_CURRENTSTATE(dmastatus);

    if ((status == DMAC_STATE_STALL) &&
        (dmac_drvctl[channel].complete == TRUE))
    {
        /* Channel/stream is aborted or complete */
        status = DMA_ABORTED;
    }
    /* Set error status if ignore is not set for channel */
    else if ((status != DMA_IDLE) &&
        ((dmastatus & DMAC_CHERROR) != 0) &&
        (dmac_drvctl[dmac_drvctl[channel].ch].ignore == FALSE))
    {
        status = DMA_ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * Function: dmac_get_packer_bytes
 *
 * Purpose: Return the number of bytes in the byte packer
 *
 * Processing:
 *     Return the number of bytes in the channel byte packer.
 *
 * Parameters:
 *     channel: DMA channel to disable or enable interrupts on
 *
 * Outputs: None
 *
 * Returns: number of bytes in the byte packer
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 dmac_get_packer_bytes(DMAC_CHANNEL_T channel)
{
    INT_32 bytesp = 0;

    /* Only perform this function if the DMA channel is allocated */
    if (dmac_drvctl[channel].alloc == TRUE)
    {
        /* Get current DMA control register */
        bytesp = DMAC_STATUS_BYTES(
            DMAC->chregs[dmac_drvctl[channel].ch].cntl_stat);
    }

    return bytesp;
}

/***********************************************************************
 *
 * Function: dmac_channel_int_enable
 *
 * Purpose: DMA interrupt enable and disable
 *
 * Processing:
 *     For the selected channel, enable or disable the interrupt in the
 *     VIC.
 *
 * Parameters:
 *     channel: DMA channel to disable or enable interrupts on
 *     enable_mask:  Non-zero to enable, 0 to disable
 *
 * Outputs: None
 *
 * Returns: Non-zero if previously enabled, 0 if previously disabled
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 dmac_channel_int_enable(DMAC_CHANNEL_T channel,
                               UNS_32 enable_mask)
{
    UNS_32 enabled;

    /* If this channel was previously enabled, set enabled to '1' */
    if (vic_int_enabled(dmac_map[dmac_drvctl[channel].ch].intid) ==
        TRUE)
    {
        enabled = 1;
    }
    else
    {
        enabled = 0;
    }

    if (enable_mask == 0)
    {
        /* Disable interrupt for this DMA channel */
        vic_int_enable(dmac_map[dmac_drvctl[channel].ch].intid, FALSE);
    }
    else
    {
        /* Enable interrupt for this DMA channel */
        vic_int_enable(dmac_map[dmac_drvctl[channel].ch].intid, TRUE);
    }

    return enabled;
}

/***********************************************************************
 *
 * Function: dmac_isr
 *
 * Purpose: DMA interrupt handler
 *
 * Processing:
 *     This function is called when a DMA interrupt occurs. Determine
 *     which DMA channel caused the interrupt by examining each
 *     channel's interrupt pending status bit. For the channel that is
 *     pending, call the dmac_state_handler() function. If an interrupt
 *     occurred for an unallocated channel, disable the channel.
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
void dmac_isr(void)
{
    DMAC_CHANNEL_MAP_T regchannel;

    /* Determine which DMA channel caused the interrupt */
    regchannel = DMAR_M2P_TX0;
    while (regchannel <= DMAR_M2P_TX9)
    {
        if ((DMAC->dma_global_int & dmac_map[regchannel].dmask) != 0)
        {
            /* Is the channel allocated? */
            if (dmac_drvctl[dma_to_periph[regchannel]].alloc == TRUE)
            {
                /* Yes, handle the interrupt */
                dmac_state_handler(dma_to_periph[regchannel]);
            }
            else
            {
                /* Not allocated, so make sure the interrupt is
                   disabled */
                vic_int_enable(dmac_map[regchannel].intid, FALSE);
            }
        }

        /* Check next channel */
        regchannel++;
    }
}
