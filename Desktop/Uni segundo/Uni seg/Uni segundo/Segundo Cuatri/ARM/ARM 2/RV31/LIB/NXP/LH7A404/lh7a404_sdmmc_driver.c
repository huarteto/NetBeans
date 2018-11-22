/***********************************************************************
 * $Id:: lh7a404_sdmmc_driver.c 14 2007-08-28 16:08:18Z kevinw         $
 *
 * Project: LH7A404 SDMMC driver
 *
 * Description:
 *     This file contains driver support for the SDMMC module on the
 *     LH7A404
 *
 * Notes:
 *     This driver supports basic MMC and SD data transfer capability
 *     and is intended to be a reference on how the SDMMC interface of
 *     the LH7A404 works. This driver can be used to transfer data to
 *     and from an SDMMC card, but not every SDMMC function is
 *     implemented and a system with an SD/MMC interface may require
 *     more capability than what this driver provides. The following is
 *     provided by this driver:
 *         Non-blocking SDMMC state management
 *         SDMMC single (interrupt) and multi-block (DMA) read and write
 *         Interrupt and DMA control and transfer modes
 *         Device status and detection (via OCR polling)
 *    Presently, this driver doesn't support more than 1 card, although
 *    the controller itself can support stacked cards. Multi-sector
 *    reads and writes require DMA to operate properly - multi-sector
 *    reads and writes are not supported in interrupt mode.
 *
 *    This driver performs state management in the interrupt handler.
 *    Some IOCTL commands that request data or status from the MMC card
 *    return to the caller without the data being available at that
 *    time. The caller can either poll the state of the SDMMC driver to
 *    determine when the command is complete, or can use the driver
 *    callback function to implement a signalling method to notify the
 *    caller when the driver has completed the request. An exception
 *    to a OK/FAIL status is the read and write commands when using DMA.
 *    Using a read/write command with DMA will return a status after
 *    the first sector is complete, but the DMA interrupt must be used
 *    to monitor the transfer completion.
 *
 *    A typical sequence for starting a transferring data on the SDMMC
 *    interface is as follows:
 *        Open the MMC driver and get the MMC driver handle
 *        Issue an IOCTL "MMC_RESET" command and wait for about 500mS
 *        Issue an IOCTL "MMC_DETECT" command
 *        Keep querying the interface for the current status
 *        If the status is an error, then a card was not installed or
 *            a detection error occurred
 *        Issue a read or write command
 *        Keep querying the interface for the current status
 *        Once the status shows complete, the data is either ready to
 *            process or has been written to the card
 *        Deselect the card
 *
 *    This driver requires interrupts and optionally DMA! The interrupt
 *    and DMA controllers must be initialized and enabled for the SDMMC
 *    outside of this driver. This driver does handle interrupt and DMA
 *    logic specific to the SDMMC interface. If only the interrupt mode
 *    is used, sector transfers will only occur one at a time.
 *
 *    The initialization command and the read/write commands when used
 *    with DMA will not call the user callback upon completion. This is
 *    by design.
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

#include "lpc_sdmmc.h"
#include "lh7a404_csc_driver.h"
#include "lh7a404_sdmmc_driver.h"

/***********************************************************************
 * SDMMC driver private types
 **********************************************************************/

/* SDMMC Block (sector) size in bytes */
#define SDMMC_BLK_SIZE    512

/* SDMMC OCR power-up complete mask (used against word 0) */
#define SDMMC_OCR_MASK    0x0080

/* SDMMC maximum number of OCR request retries before considering a
   card dead */
#define SDMMC_MAX_OCR_RET 256

/* SDMMC OCR sequence clock speed - also the default clock speed of
   the bus whenever a new card is detected and configured */
#define SDMMC_OCR_CLOCK   390000

/* Size of the SDMMC OCR word in bytes */
#define SDMMC_OCR_SIZE 6

/* Size of the SDMMC CID word in bytes */
#define SDMMC_CID_SIZE 16

/* Size of the SDMMC CSD word in bytes */
#define SDMMC_CSD_SIZE 16

/* This macro makes setting or clearing all interrupt related items
   (when all bits are set) easier */
#define SDMMC_ALL_INTS (MMC_INT_DATA_TRANSFER_DONE | \
            MMC_INT_PROGRAM_DONE | MMC_INT_END_CMD_RESPONSE | \
            MMC_INT_BUFFER_READY | MMC_INT_CLOCK_STOPPED | \
            MMC_INT_SDIO_INT)

/* Each command enumeration has a SDMMC command number (used by the
   card) and a SDMMC command/control word (used by the controller).
   This structure defines this word pair. */
typedef struct
{
    UNS_32 cmd;            /* Mapped SDMMC (spec) command */
    UNS_32 cmdctl;         /* Required bits for the SDMMC controller */
    SDMMC_RESPONSE_T resp; /* Expected response type */
} SDMMC_CMD_CTRL_T;

/* This array maps the SDMMC command enumeration to the hardware SDMMC
   command index, the controller setup value, and the expected response
   type */
SDMMC_CMD_CTRL_T sdmmc_cmds[SDMMC_INVALID_CMD] =
{
    /* SDMMC_IDLE - this command has a response type of none, but the
       controller will be spoofed into thinking it has another response
       type so we can use the response timeout interrupt */
    {0,  MMC_CMD_INITIALIZE,   SDMMC_RESPONSE_R1},
    /* MMC_SENDOP_COND */
    {1,  MMC_CMD_INITIALIZE,   SDMMC_RESPONSE_R3},
    /* SDMMC_ALL_SEND_CID */
    {2,  0,                    SDMMC_RESPONSE_R2},
    /* SDMMC_SRA */
    {3,  0,                    SDMMC_RESPONSE_R1},
    /* MMC_PROGRAM_DSR */
    {4,  0,                    SDMMC_RESPONSE_NONE},
    /* SDMMC_SELECT_CARD */
    {7,  0,                    SDMMC_RESPONSE_R1},
    /* SDMMC_SEND_CSD */
    {9,  0,                    SDMMC_RESPONSE_R2},
    /* SDMMC_SEND_CID */
    {10, 0,                    SDMMC_RESPONSE_R2},
    /* SDMMC_READ_UNTIL_STOP */
    {11, (MMC_CMD_DATA_EN | MMC_CMD_BUSY | MMC_CMD_STREAM),
                               SDMMC_RESPONSE_R1},
    /* SDMMC_STOP_XFER */
    {12, MMC_CMD_BUSY,         SDMMC_RESPONSE_R1},
    /* SDMMC_SSTAT */
    {13, 0,                    SDMMC_RESPONSE_R1},
    /* SDMMC_INACTIVE - this command has a response type of none, but
       the controller will be spoofed into thinking it has another
       response type so we can use the response timeout interrupt */
    {15, 0,                    SDMMC_RESPONSE_R1},
    /* SDMMC_SET_BLEN */
    {16, 0,                    SDMMC_RESPONSE_R1},
    /* SDMMC_READ_SINGLE */
    {17, (MMC_CMD_DATA_EN | MMC_CMD_BIG_ENDIAN),
                               SDMMC_RESPONSE_R1},
    /* SDMMC_READ_MULTIPLE */
    {18, (MMC_CMD_DATA_EN | MMC_CMD_BIG_ENDIAN),
                               SDMMC_RESPONSE_R1},
    /* SDMMC_WRITE_UNTIL_STOP */
    {20, (MMC_CMD_DATA_EN | MMC_CMD_BUSY | MMC_CMD_STREAM |
          MMC_CMD_WRITE),      SDMMC_RESPONSE_R1},
    /* SDMMC_SET_BLOCK_COUNT */
    {23, 0,                    SDMMC_RESPONSE_R1},
    /* SDMMC_WRITE_SINGLE */
    {24, (MMC_CMD_DATA_EN | MMC_CMD_BIG_ENDIAN | MMC_CMD_WRITE),
                               SDMMC_RESPONSE_R1},
    /* SDMMC_WRITE_MULTIPLE */
    {25, (MMC_CMD_DATA_EN | MMC_CMD_BIG_ENDIAN | MMC_CMD_WRITE),
                               SDMMC_RESPONSE_R1},
    /* MMC_PROGRAM_CID */
    {26, 0,                    SDMMC_RESPONSE_R1},
    /* SDMMC_PROGRAM_CSD */
    {27, 0,                    SDMMC_RESPONSE_R1},
    /* SDMMC_SET_WR_PROT */
    {28, 0,                    SDMMC_RESPONSE_R1B},
    /* SDMMC_CLEAR_WR_PROT */
    {29, 0,                    SDMMC_RESPONSE_R1B},
    /* SDMMC_SEND_WR_PROT */
    {30, 0,                    SDMMC_RESPONSE_R1},
    /* SD_ERASE_BLOCK_START */
    {32, 0,                    SDMMC_RESPONSE_R1},
    /* SD_ERASE_BLOCK_END */
    {33, 0,                    SDMMC_RESPONSE_R1},
    /* MMC_ERASE_BLOCK_START */
    {35, 0,                    SDMMC_RESPONSE_R1},
    /* MMC_ERASE_BLOCK_END */
    {36, 0,                    SDMMC_RESPONSE_R1},
    /* MMC_ERASE_BLOCKS */
    {38, 0,                    SDMMC_RESPONSE_R1B},
    /* MMC_FAST_IO */
    {39, 0,                    SDMMC_RESPONSE_R4},
    /* MMC_GO_IRQ_STATE */
    {40, 0,                    SDMMC_RESPONSE_R5},
    /* MMC_LOCK_UNLOCK */
    {42, 0,                    SDMMC_RESPONSE_R1B},
    /* SDMMC_APP_CMD */
    {55, 0,                    SDMMC_RESPONSE_R1},
    /* SDMMC_GEN_CMD */
    {56, 0,                    SDMMC_RESPONSE_R1B}
};

/* This array maps the SDMMC application specific command enumeration to
   the hardware SDMMC command index, the controller setup value, and the
   expected response type */
SDMMC_CMD_CTRL_T sdmmc_app_cmds[SD_INVALID_APP_CMD] =
{
    /* SD_SET_BUS_WIDTH */
    {6,  0,                    SDMMC_RESPONSE_R1},
    /* SD_SEND_STATUS */
    {13, 0,                    SDMMC_RESPONSE_R1},
    /* SD_SEND_WR_BLOCKS */
    {22, 0,                    SDMMC_RESPONSE_R1},
    /* SD_SET_ERASE_COUNT */
    {23, 0,                    SDMMC_RESPONSE_R1},
    /* SD_SENDOP_COND */
    {41, MMC_CMD_INITIALIZE,   SDMMC_RESPONSE_R3},
    /* SD_CLEAR_CARD_DET */
    {42, 0,                    SDMMC_RESPONSE_R1},
    /* SD_SEND_SCR */
    {51, 0,                    SDMMC_RESPONSE_R1}
};

/* This array maps the reponse enumeration type to an 'OR'ed value to
   be used with the cmdctl word - the controller only supports response
   type sizes of R1 through R3 (and none), so some repsonses (for
   example R6), are based on these sizes */
UNS_32 sdmmc_r1add[SDMMC_RESPONSE_NONE + 1] =
{
    MMC_CMD_RESPONSE_R1,
    MMC_CMD_RESPONSE_R1,     /* Response R1b is the size of R1 */
    MMC_CMD_RESPONSE_R2,
    MMC_CMD_RESPONSE_R3,
    MMC_CMD_RESPONSE_R1,     /* Response R4 is the size of R1 */
    MMC_CMD_RESPONSE_R1,     /* Response R5 is the size of R1 */
    MMC_CMD_RESPONSE_R1,     /* Response R6 is the size of R1 */
    MMC_CMD_RESPONSE_NONE
};

/* The interrupt logic is maintained by a state engine that cycles
   through different steps to perform an SDMMC function. This
   enumeration specifies the major state that the SDMMC driver is
   in */
typedef enum
{
    SDMMC_NO_CARD,         /* No card has been detected yet */
    SDMMC_RESET,           /* Card reset state */
    SDMMC_INIT_SD_STATE,   /* In the SD card initialization state */
    SDMMC_INIT_MMC_STATE,  /* In the MMC card initialization state */
    SDMMC_REG_FETCH,       /* SDMMC driver is requesting a register */
    SDMMC_READ_STATE,      /* SDMMC driver is requesting a block read */
    SDMMC_WRITE_STATE,     /* SDMMC driver is requesting a block
                              write */
    SDMMC_DMA,             /* SDMMC driver is waiting for the closure of
                              a DMA transfer */
    SDMMC_DESELECT,        /* Deslect card and stop clock */
    SDMMC_EJECTING,        /* Card is shutting down (ejecting) */
    SDMMC_READY            /* Driver is ready for a command */
} SDMMC_MAJOR_STATE_T;

/* Main SDMMC driver data structure */
typedef struct
{
    BOOL_32 init;             /* Driver initialized flag */
    SDMMC_REGS_T *regptr;     /* Pointer to MMC registers */
    UNS_16 saved_ocr[SDMMC_OCR_SIZE / 2]; /* Saved OCR data */
    UNS_16 saved_cid[SDMMC_CID_SIZE / 2]; /* Saved CID data */
    INT_32 sd_card;           /* SD card mode flag */
    PFV sdmmc_cb;             /* User callback function pointer */
    UNS_16 rca;               /* Relative card address */
    UNS_16 *buffer;           /* Buffer pointer used for various
                                 transfer functions */
    INT_32 bsize;             /* Size of the transfer or number of
                                 sectors to transfer */
    INT_32 to_transfer;       /* Number of sectors left to transfer */
    UNS_32 sector_start;      /* Starting sector to read/write */
    BOOL_32 use_dma;          /* Use DMA for transfer of data instead
                                 of a copy loop */

    /* Driver state control data */    
    SDMMC_MAJOR_STATE_T state; /* Current state the driver is in */
    INT_32 state_step;        /* Current step for the major state */
    SDMMC_CMD_STATUS_T lstat; /* Status from last IOCTL command */

    /* These variables are used in initialization only */
    INT_32 ocr_attempts;      /* Number of OCR request attempts left */
    UNS_32 ocrval;            /* Saved OCR request value */
} SDMMC_CFG_T;

/* SDMMC driver data structure */
SDMMC_CFG_T sdmmccfg;

/***********************************************************************
 * MMC driver private functions
 **********************************************************************/

/* Forward declaration of main state function */
void sdmmc_state_machine(SDMMC_CFG_T *sdmmccfgptr);

/***********************************************************************
 *
 * Function: sdmmc_set_clock
 *
 * Purpose: Compute the closest SDMMC clock frequency in Hz and set it
 *
 * Processing:
 *     All possible combinations of predivider and main divider are
 *     used to determine the closest matching MMC clock rate that will
 *     be generated based on the passed desired rate.
 *
 * Parameters:
 *     sdmmc_regs: Pointer to SDMMC register set
 *     hz: SDMMC clock frequency in Hz
 *
 * Outputs: None
 *
 * Returns: The new programmed frequency
 *
 * Notes:
 *     The SDMMC clock should be disabled prior to using this function.
 *
 **********************************************************************/
UNS_32 sdmmc_set_clock(SDMMC_REGS_T *sdmmc_regs,
                       UNS_32 hz)
{
    UNS_32 svclk, bus_hz, mmc_div_val, mmc_div_program_val;
    UNS_32 mmc_prediv_program_val, comp_freq;
    UNS_32 saved_prediv, saved_div;

    /* Find the closest divider to get the desired clock frequency */
    bus_hz = csc_get_clock(AHBCLK);
    saved_prediv = 8;
    saved_div = 64;
    svclk = bus_hz / (saved_div * saved_prediv);

    /* Loop through all frequency combinations and find the closest
       match */
    mmc_div_val = 1;
    for (mmc_div_program_val = MMC_CLOCK_DIV1;
        mmc_div_program_val <= MMC_CLOCK_DIV64; mmc_div_program_val++)
    {
        /* Loop through each predivider combination */
        for (mmc_prediv_program_val = 1;
                mmc_prediv_program_val <= 8; mmc_prediv_program_val++)
        {
            /* Is the calculated frequency going to be less than or
               equal to what is requested? */
            comp_freq = bus_hz / (mmc_div_val * mmc_prediv_program_val);
            if (comp_freq <= hz)
            {
                /* Is this frequency closer to the last calculation? */
                if ((hz - svclk) > (hz - comp_freq))
                {
                    /* This is a closer match */
                    svclk = comp_freq;

                    /* Save fitted values */
                    saved_prediv = mmc_prediv_program_val;
                    saved_div = mmc_div_program_val;

                    /* Force skip to next main divider */
                    mmc_prediv_program_val = 9;
                }
            }
        }

        mmc_div_val = mmc_div_val * 2;
    }

    /* Set current dividers to get the MMC clock */
    sdmmc_regs->clock_rate = saved_div;
    saved_div = sdmmc_regs->clock_prediv & ~MMC_PREDIV(0xF);
    sdmmc_regs->clock_prediv = saved_div | MMC_PREDIV(saved_prediv);

    /* Ideally, these timings can be adjusted based on requested
       clock speed and CSD data, but are locked at a reasonable speed
       for now. Since they are only typically used in an error
       condition, they should not effect performance */
    sdmmc_regs->response_to = 0x3F;
    sdmmc_regs->read_to = 0x7FFF;

    return svclk;
}

/***********************************************************************
 *
 * Function: sdmmc_clock_stop
 *
 * Purpose: Stops the SDMMC clock
 *
 * Processing:
 *     Stop the SDMMC clock by disabling it in the SDMMC clock control.
 *     Wait for the clock to stop.
 *
 * Parameters:
 *     sdmmc_regs: Pointer to SDMMC register set
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void sdmmc_clock_stop(SDMMC_REGS_T *sdmmc_regs)
{
    /* Stop the clock */
    sdmmc_regs->clock_control = MMC_CTRL_STOP_CLK;

    /* Wait for the clock to stop */
    while ((sdmmc_regs->status & MMC_STATUS_CLOCK_STOPPED) == 0);
}

/***********************************************************************
 *
 * Function: sdmmc_clock_start
 *
 * Purpose: Starts the SDMMC clock
 *
 * Processing:
 *     Starts the SDMMC clock by enabling it in the SDMMC clock control.
 *
 * Parameters:
 *     sdmmc_regs: Pointer to SDMMC register set
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None 
 *
 **********************************************************************/
void sdmmc_clock_start(SDMMC_REGS_T *sdmmc_regs)
{
    /* Start the clock */
    sdmmc_regs->clock_control = MMC_CTRL_START_CLK;
}

/***********************************************************************
 *
 * Function: sdmmc_command
 *
 * Purpose: Send the SDMMC command to the card
 *
 * Processing:
 *     Stop the SDMMC clock. Fetch and save the mapped command for the
 *     passed command enumeration in the SDMMC command register. Save
 *     the passed argument in the SDMMC argument register. Fetch the
 *     SDMMC control word for the passed command enumeration and OR
 *     in the desired response value. If the current card type is an
 *     SD card, enable wide more on the control word. Save the control
 *     word in the SDMMC control word register. Enable the SDMMC clock
 *     to start the configured operation.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *     cmd: Basic SDMMC command to send
 *     arg: SDMMC command argument
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None 
 *
 **********************************************************************/
void sdmmc_command(SDMMC_CFG_T *sdmmccfgptr,
                   UNS_32 cmd,
                   UNS_32 arg)
{
    SDMMC_REGS_T *sdmmc_regs = sdmmccfgptr->regptr;
    UNS_32 cmdctl;

    /* Stop the clock */
    sdmmc_clock_stop(sdmmc_regs);

    /* Place the command and argument in the command number and
       argument registers */
    sdmmc_regs->command = sdmmc_cmds[cmd].cmd;
    sdmmc_regs->argument = arg;

    /* Update controller command/control word */
    cmdctl = sdmmc_cmds[cmd].cmdctl |
        sdmmc_r1add [sdmmc_cmds[cmd].resp];
    if (sdmmccfgptr->sd_card != 0)
    {
        /* If this is an SD card, switch to a 4-bit bus */
        cmdctl = cmdctl | MMC_CMD_WIDE;
    }

    sdmmc_regs->cmd_control = cmdctl;

    /* Start the clock */
    sdmmc_clock_start(sdmmc_regs);
}

/***********************************************************************
 *
 * Function: sd_command
 *
 * Purpose: Send the SD specific command to the card (SD only)
 *
 * Processing:
 *     This function performs the same task as the sdmmc_command()
 *     except the fetch command word is an SD ACMD value.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *     cmd: Basic SD command to send
 *     arg: SD command argument
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None 
 *
 **********************************************************************/
void sd_command(SDMMC_CFG_T *sdmmccfgptr,
                UNS_32 cmd,
                UNS_32 arg)
{
    SDMMC_REGS_T *sdmmc_regs = sdmmccfgptr->regptr;
    UNS_32 cmdctl;

    /* Stop the clock */
    sdmmc_clock_stop(sdmmc_regs);

    /* Place the command and argument in the command number and
       argument registers */
    sdmmc_regs->command = sdmmc_app_cmds[cmd].cmd;
    sdmmc_regs->argument = arg;

    /* Update controller command/control word */
    cmdctl = sdmmc_app_cmds[cmd].cmdctl |
        sdmmc_r1add [sdmmc_app_cmds[cmd].resp];
    if (sdmmccfgptr->sd_card != 0)
    {
        /* If this is an SD card, switch to a 4-bit bus */
        cmdctl = cmdctl | MMC_CMD_WIDE;
    }

    sdmmc_regs->cmd_control = cmdctl;

    /* Start the clock */
    sdmmc_clock_start(sdmmc_regs);
}

/***********************************************************************
 *
 * Function: sdmmc_swap_bytes
 *
 * Purpose: Swaps the MMC response byte halves
 *
 * Processing:
 *     Swap the upper and lower 8-bit byte halves of the passed 16-bit
 *     word.
 *
 * Parameters:
 *     data: Pointer to 16-bit word to swap halves on
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     This function is needed for the response and data FIFOS to
 *     reorder the bytes in small endian format. The data FIFO doesn't
 *     need this function if the BIG_ENDIAN bit is set in the control
 *     word.
 *
 **********************************************************************/
void sdmmc_swap_bytes(UNS_16 *data)
{
    UNS_8 temp, *temp_ptr = (UNS_8 *) data;

    temp = *temp_ptr;
    *temp_ptr = *(temp_ptr + 1);
    *(temp_ptr + 1) = temp;
}

/***********************************************************************
 *
 * Function: sdmmc_get_response
 *
 * Purpose: Reads the SDMMC commands response
 *
 * Processing:
 *     For the requested response size, read the response FIFO into the
 *     passed buffer. Swap the upper and lower bytes of each word value
 *     in the buffer.
 *
 * Parameters:
 *     sdmmc_regs: Pointer to SDMMC register set
 *     buf      : Pointer to where the put the response data
 *     resp_size: Number of bytes to read from the response FIFO
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None 
 *
 **********************************************************************/
void sdmmc_get_response(SDMMC_REGS_T *sdmmc_regs,
                        void *buf,
                        INT_32 resp_size)
{
    UNS_16 *response;
    INT_32 idx;

    response = (UNS_16 *) buf;

    /* Read the necessary number of response words from the response
       FIFO */
    for (idx = 0; idx < (resp_size / 2); idx++)
    {
        *response = sdmmc_regs->response_fifo;
        sdmmc_swap_bytes(response);
        response++;
    }
}

/***********************************************************************
 *
 * Function: sd_init_state_manager
 *
 * Purpose: SD card initialization sequence state manager
 *
 * Processing:
 *     This function performs the necessary state logic to initialize
 *     an SD card connected to the controller. This logic is per the
 *     SD/MMC specification. If the card is not an SD card checked with
 *     the CMD55 (APP) command (which an uninitialized MMC card will not
 *     response to), then the card is assumed to be an SD card.
 *     Detection will drop out and return a failure status (and leave
 *     the driver is a no card state) if any step of the initialization
 *     sequence fails.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: TRUE if the sequence is finished
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 sd_init_state_manager(SDMMC_CFG_T *sdmmccfgptr)
{
    UNS_16 resp_r1[16];
    INT_32 idx;
    BOOL_32 retry, finished = FALSE;
    SDMMC_REGS_T *sdmmc_regs = sdmmccfgptr->regptr;

    switch (sdmmccfgptr->state_step)
    {
        case 0:
            /* Step 0: Set the number of OCR request attempts before
               giving up and switch to the next state */
            sdmmccfgptr->ocr_attempts = SDMMC_MAX_OCR_RET;
            sdmmccfgptr->state_step++;

            /* Clear and enable command/response interrupt */
            sdmmc_regs->int_clear = SDMMC_ALL_INTS;
            sdmmc_regs->int_enable = MMC_INT_END_CMD_RESPONSE;

        case 1:
            /* Step 1: Check the OCR request count and if it is not
               0, perform an APP command to the card and decrement
               the OCR request count */
            sdmmccfgptr->lstat = MMC_WAITING_FOR_RESPONSE;
            if (sdmmccfgptr->ocr_attempts > 0)
            {
                sdmmccfgptr->ocr_attempts--;
                sdmmccfgptr->state_step++;
                sdmmc_command(sdmmccfgptr, SDMMC_APP_CMD, 0);
            }
            else
            {
                /* No more requests, this card didn't initialize
                   correctly */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
                sdmmccfgptr->state = SDMMC_NO_CARD;
                finished = TRUE;
            }
            break;

        case 2:
            /* Step 2: If the response from the controller didn't time
               out, then clear the reponse buffer and issue the send
               operational condition command */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Clear out the response FIFO */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);

                /* Good response, send the OCR command */
                sdmmccfgptr->state_step++;
                sd_command(sdmmccfgptr, SD_SENDOP_COND,
                    sdmmccfgptr->ocrval);
            }
            else
            {
                /* Response timed out, try this as an MMC card
                   instead */
                sdmmccfgptr->ocr_attempts = SDMMC_MAX_OCR_RET;
                sdmmccfgptr->state_step = 0;
                sdmmccfgptr->state = SDMMC_INIT_MMC_STATE;
                sdmmc_state_machine(sdmmccfgptr);
            }
            break;

        case 3:
            /* Step 3: Read the returned response and check it against
               the passed OCR word. If it matches, check the ready bit
               and continue processing if ready, otherwise try step 1
               again */
            retry = TRUE;
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Fetch and check the OCR word */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0],
                    SDMMC_OCR_SIZE);
                if ((resp_r1[0] & SDMMC_OCR_MASK) != 0)
                {
                    /* Ready, go to next state */
                    sdmmccfgptr->state_step++;

                    /* Save OCR */
                    for (idx = 0; idx < (SDMMC_OCR_SIZE / 2); idx++)
                    {
                        sdmmccfgptr->saved_ocr[idx] = resp_r1[idx];
                    }

                    /* No retry needed */
                    retry = FALSE;

                    /* Do an ALL send CID */
                    sdmmc_command(sdmmccfgptr, SDMMC_ALL_SEND_CID, 0);
                }
            }

            /* Is a retry needed? */
            if (retry == TRUE)
            {
                sdmmccfgptr->state_step = 1;
                finished = sd_init_state_manager(sdmmccfgptr);
            }
            break;
 
        case 4:
            /* Step 4: Make sure the reponse is valid. If it is, throw
               it away and get the relative address of the card */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0],
                    SDMMC_CID_SIZE);

                /* Save CID */
                for (idx = 0; idx < (SDMMC_CID_SIZE / 2); idx++)
                {
                    sdmmccfgptr->saved_cid[idx] = resp_r1[idx];
                }

                /* Set the relative card address */
                sdmmccfgptr->state_step++;
                sdmmc_command(sdmmccfgptr, SDMMC_SRA, 0);
            }
            else
            {
                /* No response, stop here with an error */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
                sdmmccfgptr->state = SDMMC_NO_CARD;
                finished = TRUE;
            }
            break;

         case 5:
            /* Step 5: Make sure the reponse is valid. If it is, get
               the RCA from the status and select this card */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Get response and returned RCA address */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);

                /* Save RCA address from SD card */
                sdmmccfgptr->rca = ((resp_r1[0] & 0x00FF) << 8) |
                    ((resp_r1[1] & 0xFF00) >> 8);

                /* Select this card */
                sdmmccfgptr->state_step++;
                sdmmc_command(sdmmccfgptr, SDMMC_SELECT_CARD,
                    (sdmmccfgptr->rca << 16));
            }
            else
            {
                /* No response, stop here with an error */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
                sdmmccfgptr->state = SDMMC_NO_CARD;
                finished = TRUE;
            }
            break;

        case 6:
            /* Step 6: Make sure the reponse is valid. If it is, get
               the RCA from the status */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);

                /* Send APP Command */
                sdmmccfgptr->state_step++;
                sdmmc_command(sdmmccfgptr, SDMMC_APP_CMD,
                    (sdmmccfgptr->rca << 16));
            }
            else
            {
                /* No response, stop here with an error */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
                sdmmccfgptr->state = SDMMC_NO_CARD;
                finished = TRUE;
            }
            break;

         case 7:
            /* Step 7: Make sure the reponse is valid. If it is, throw
               it away and issue the set bus width command to set the
               bus size to 4-bits. Set the SD mode flag to 1. */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);

                /* Issue bus width command at 16 bits */
                sdmmccfgptr->state_step++;
                sd_command(sdmmccfgptr, SD_SET_BUS_WIDTH, 2);
            }
            else
            {
                /* No response, stop here with an error */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
                sdmmccfgptr->state = SDMMC_NO_CARD;
                finished = TRUE;
            }
            break;

         case 8:
            /* Step 8: Make sure the reponse is valid. If it is, throw
               it away and set the status and finished flag */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);

                /* No response, stop here with an error */
                sdmmccfgptr->lstat = MMC_OPERATION_OK;

                /* New card state is SDMMC_READY */
                sdmmccfgptr->state = SDMMC_READY;

                /* SD card mode enabled */
                sdmmccfgptr->sd_card = 1;
            }
            else
            {
                /* No response, stop here with an error */
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;

                /* New card state is SDMMC_NO_CARD */
                sdmmccfgptr->state = SDMMC_NO_CARD;
            }

            /* Leave the clock and interrupts disabled */
            sdmmc_regs->int_enable = 0;
            sdmmc_clock_stop(sdmmccfgptr->regptr);
            finished = TRUE;
            break;

        default:
            break;
    }

    return finished;
}

/***********************************************************************
 *
 * Function: mmc_init_state_manager
 *
 * Purpose: MMC card initialization sequence state manager
 *
 * Processing:
 *     This function performs the necessary state logic to initialize
 *     an MMC card connected to the controller. This logic is per the
 *     MMC specification. Detection will drop out and return a failure
 *     status (and leave the driver is a no card state) if any step of
 *     the initialization sequence fails.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: TRUE if the sequence is finished
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 mmc_init_state_manager(SDMMC_CFG_T *sdmmccfgptr)
{
    UNS_16 resp_r1[16];
    INT_32 idx;
    BOOL_32 retry, finished = FALSE;
    SDMMC_REGS_T *sdmmc_regs = sdmmccfgptr->regptr;

    switch (sdmmccfgptr->state_step)
    {
        case 0:
            /* Step 0: Set the number of OCR request attempts before
               giving up and switch to the next state */
            sdmmccfgptr->ocr_attempts = SDMMC_MAX_OCR_RET;
            sdmmccfgptr->state_step++;

            /* Clear and enable command/response interrupt */
            sdmmc_regs->int_clear = SDMMC_ALL_INTS;
            sdmmc_regs->int_enable = MMC_INT_END_CMD_RESPONSE;

        case 1:
            /* Step 1: Check the OCR request count and if it is not
               0, perform an Send OP command to the card and decrement
               the OCR request count */
            sdmmccfgptr->lstat = MMC_WAITING_FOR_RESPONSE;
            if (sdmmccfgptr->ocr_attempts > 0)
            {
                sdmmccfgptr->ocr_attempts--;
                sdmmccfgptr->state_step++;
                sdmmc_command(sdmmccfgptr, MMC_SENDOP_COND,
                    sdmmccfgptr->ocrval);
            }
            else
            {
                /* No more requests, this card didn't not initialize
                   correctly */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
                sdmmccfgptr->state = SDMMC_NO_CARD;
                finished = TRUE;
            }
            break;

        case 2:
            /* Step 2: Read the returned response and check the ready
               bit. If the ready bit is set, continue processing,
               otherwise try step 1 again */
            retry = TRUE;
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Fetch and check the OCR word */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0],
                    SDMMC_OCR_SIZE);
                if ((resp_r1[0] & SDMMC_OCR_MASK) != 0)
                {
                    /* Good match, check the ready bit */
                    if ((resp_r1[0] & SDMMC_OCR_MASK) != 0)
                    {
                        /* Ready, go to next state */
                        sdmmccfgptr->state_step++;

                        /* Save OCR */
                        for (idx = 0; idx < (SDMMC_OCR_SIZE / 2); idx++)
                        {
                            sdmmccfgptr->saved_ocr[idx] = resp_r1[idx];
                        }

                        /* No retry needed */
                        retry = FALSE;

                        /* Do an ALL send CID */
                        sdmmc_command(sdmmccfgptr,
                            SDMMC_ALL_SEND_CID, 0);
                    }
                }
            }

            /* Is a retry needed? */
            if (retry == TRUE)
            {
                sdmmccfgptr->state_step = 1;
                finished = mmc_init_state_manager(sdmmccfgptr);
            }
            break;

        case 3:
            /* Step 3: Make sure the reponse is valid. If it is, throw
               it away and set the relative address of the card */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0],
                    SDMMC_CID_SIZE);

                /* Save CID */
                for (idx = 0; idx < (SDMMC_CID_SIZE / 2); idx++)
                {
                    sdmmccfgptr->saved_cid[idx] = resp_r1[idx];
                }

                /* Set the relative card address */
                sdmmccfgptr->state_step++;
                sdmmc_command(sdmmccfgptr, SDMMC_SRA,
                    (sdmmccfgptr->rca << 16));
            }
            else
            {
                /* No response, stop here with an error */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
                sdmmccfgptr->state = SDMMC_NO_CARD;
                finished = TRUE;
            }
            break;
 
         case 4:
            /* Step 4: Select the card */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0],
                    SDMMC_CID_SIZE);

                /* Select the card */
                sdmmccfgptr->state_step++;
                sdmmc_command(sdmmccfgptr, SDMMC_SELECT_CARD,
                    (sdmmccfgptr->rca << 16));
            }
            else
            {
                /* No response, stop here with an error */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
                sdmmccfgptr->state = SDMMC_NO_CARD;
                finished = TRUE;
            }
            break;

         case 5:
            /* Step 5: Make sure the reponse is valid. If it is, throw
               it away and set the status and finished flag */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0], 4);

                /* No response, stop here with an error */
                sdmmccfgptr->lstat = MMC_OPERATION_OK;

                /* New card state is SDMMC_READY */
                sdmmccfgptr->state = SDMMC_READY;
            }
            else
            {
                /* No response, stop here with an error */
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;

                /* New card state is SDMMC_NO_CARD */
                sdmmccfgptr->state = SDMMC_NO_CARD;
            }

            /* Leave the clock and interrupts disabled */
            sdmmc_regs->int_enable = 0;
            sdmmc_clock_stop(sdmmccfgptr->regptr);
            finished = TRUE;
            break;

        default:
            break;
    }
    return finished;
}

/***********************************************************************
 *
 * Function: sdmmc_read_request_manager
 *
 * Purpose: SDMMC read request state manager
 *
 * Processing:
 *     This state machine starts the read request on the SDMMC
 *     controller. The read sequence included card selection, block
 *     sizing, the start of the sector data transfer, response check,
 *     and request logic completion status.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: TRUE if the read request sequence is finished
 *
 * Notes:
 *     When transferring data using DMA, the transaction response will
 *     be successful after the first data sector is transferred. The
 *     DMA driver must be aware of the completion of the transfer
 *     instead of this driver.
 *
 **********************************************************************/
BOOL_32 sdmmc_read_request_manager(SDMMC_CFG_T *sdmmccfgptr)
{
    UNS_16 tmp16, resp_r1[16];
    UNS_8 *buffin;
    INT_32 idx;
    UNS_32 tmp;
    BOOL_32 finished = FALSE;
    SDMMC_REGS_T *sdmmc_regs = sdmmccfgptr->regptr;

    switch (sdmmccfgptr->state_step)
    {
        case 0:
            /* Step 0: set block size */

            /* Does the data FIFO have any data in it? */
            if ((sdmmc_regs->status & MMC_STATUS_FIFO_EMPTY) == 0)
            {
                /* Yes, enable read FIFOS for the clear of data FIFO */
                sdmmc_regs->clock_prediv |= MMC_PREDIV_APB_RD_EN;

                /* Pre-empty the data FIFO */
                while ((sdmmc_regs->status & MMC_STATUS_FIFO_EMPTY) == 0)
                {
                    resp_r1[0] = sdmmc_regs->data_fifo;
                }
            }

            /* If DMA is enabled for this transfer, disable the FIFO
               peripheral interface */
            tmp = sdmmc_regs->clock_prediv;
            if (sdmmccfgptr->use_dma == TRUE)
            {
                /* Disable read FIFOS (needed with DMA) */
                tmp = tmp & ~MMC_PREDIV_APB_RD_EN;
            }
            else
            {
                /* Enable read FIFOS */
                tmp = tmp | MMC_PREDIV_APB_RD_EN;
            }
            sdmmc_regs->clock_prediv = tmp;

            /* Default status is waiting for response */
            sdmmccfgptr->lstat = MMC_WAITING_FOR_RESPONSE;
            sdmmccfgptr->state_step++;

            /* Setup block count */
            sdmmc_command(sdmmccfgptr, SDMMC_SET_BLEN,
                (sdmmccfgptr->bsize << 9));
            sdmmc_regs->block_count = sdmmccfgptr->bsize;

            /* Save sectors to transfer count down value */
            sdmmccfgptr->to_transfer = sdmmccfgptr->bsize;

            /* Enable the response interrupt */
            sdmmc_regs->int_enable = MMC_INT_END_CMD_RESPONSE;
            break;

        case 1:
            /* Step 1: If the response was good, then start the
               interrupt */
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);
                sdmmccfgptr->state_step++;

                /* Enable the data interrupt */
                sdmmc_regs->int_clear = MMC_INT_DATA_TRANSFER_DONE;

                /* Start data read request */
                if (sdmmccfgptr->bsize > 1)
                {
                    /* Issue multi-block read command - this will only
                       work with DMA and will not get a data response
                       interrupt after the transfer. It is the
                       responsibility of the DMA transfer function to
                       check the data transfer flag and data CRC flag
                       and re-issue the request if needed! */
                    sdmmc_regs->int_enable = MMC_INT_END_CMD_RESPONSE;
                    sdmmc_command(sdmmccfgptr, SDMMC_READ_MULTIPLE,
                        (sdmmccfgptr->sector_start << 9));

                    /* Set states for DMA */
                    sdmmccfgptr->lstat = MMC_DMA;
                    sdmmccfgptr->state = SDMMC_DMA;
                }
                else
                {
                    /* Issue single block read command */
                    sdmmc_regs->int_enable =
                        (MMC_INT_DATA_TRANSFER_DONE |
                        MMC_INT_END_CMD_RESPONSE);
                    sdmmc_command(sdmmccfgptr, SDMMC_READ_SINGLE,
                        (sdmmccfgptr->sector_start << 9));
                }
            }
            else
            {
                /* No response, stop here with an error */
                sdmmc_regs->int_enable = 0;
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
                sdmmccfgptr->state = SDMMC_READY;
                finished = TRUE;
            }

            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            break;

        case 2:
            /* Step 2: This is a data response and transfer step */

            /* For a data transfer, the response is only returned once
               at the start of the data transfer - it may actually occur
               before the transfer is complete, so this step may occur
               more than once */
            if ((sdmmc_regs->status &
                MMC_STATUS_END_COMMAND_RESPONSE) != 0)
            {
                if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) != 0)
                {
                    /* Response timed out, something is wrong, stop
                       here */
                    sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
                    sdmmccfgptr->state = SDMMC_READY;
                    finished = TRUE;
                }
                else
                {
                    /* Just throw away response */
                    sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);

                    /* If DMA mode, set the finished flag now */
                    if (sdmmccfgptr->state == SDMMC_DMA)
                    {
                        finished = TRUE;
                    }
                }

                /* Clear the command response interrupt */
                sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            }

            if ((finished == FALSE) && ((sdmmc_regs->status &
                MMC_STATUS_DATA_TRANSFER_DONE) != 0))
            {
                if ((sdmmc_regs->status & MMC_STATUS_READ_TO) != 0)
                {
                    /* Data read timed out */
                    sdmmccfgptr->state = SDMMC_READY;
                    sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
                    finished = TRUE;
                }
                else
                {
                    /* The FIFO is full, empty it */
                    buffin = (UNS_8 *) sdmmccfgptr->buffer;
                    for (idx = 0; idx < (SDMMC_BLK_SIZE / 2); idx++)
                    {
                        tmp16 = sdmmc_regs->data_fifo;
                        *buffin = (UNS_8) (tmp16 & 0xFF);
                        buffin++;
                        *buffin = (UNS_8) (tmp16 >> 8);
                        buffin++;
                    }

                    /* If a data CRC error occurs, set the read
                       error flag */
                    if ((sdmmc_regs->status &
                        MMC_STATUS_CRC_ERROR_READ) != 0)
                    {
                        sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
                    }
                    else
                    {
                        sdmmccfgptr->lstat = MMC_OPERATION_OK;
                    }

                    sdmmccfgptr->state = SDMMC_READY;
                    finished = TRUE;
                }

                /* Clear the data interrupt */
                sdmmc_regs->int_clear = MMC_INT_DATA_TRANSFER_DONE;
            }

            if (finished == TRUE)
            {
                sdmmc_regs->int_enable = 0;
            }

            break;

        default:
            /* There should be no other cases */
            break;
    }

    return finished;
}

/***********************************************************************
 *
 * Function: sdmmc_write_request_manager
 *
 * Purpose: SDMMC write request state manager
 *
 * Processing:
 *     This state machine starts the write request on the SDMMC
 *     controller. The write sequence included card selection, block
 *     sizing, the start of the sector data transfer, response check,
 *     and request logic completion status.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: TRUE if the write request sequence is finished
 *
 * Notes:
 *     When transferring data using DMA, the transaction response will
 *     be successful after the first data sector is transferred. The
 *     DMA driver must be aware of the completion of the transfer
 *     instead of this driver.
 *
 **********************************************************************/
BOOL_32 sdmmc_write_request_manager(SDMMC_CFG_T *sdmmccfgptr)
{
    UNS_16 *buffin, resp_r1[16];
    INT_32 idx;
    UNS_32 tmp;
    BOOL_32 finished = FALSE;
    SDMMC_REGS_T *sdmmc_regs = sdmmccfgptr->regptr;

    switch (sdmmccfgptr->state_step)
    {
        case 0:
            /* Step 0: Set block size */

            /* Enable read FIFOS for the clear of data FIFO */
            sdmmc_regs->clock_prediv |= MMC_PREDIV_APB_RD_EN;

            /* Pre-empty the data FIFO */
            while ((sdmmc_regs->status & MMC_STATUS_FIFO_EMPTY) == 0)
            {
                resp_r1[0] = sdmmc_regs->data_fifo;
            }

            /* If DMA is enabled for this transfer, disable the FIFO
               peripheral interface */
            if (sdmmccfgptr->use_dma == TRUE)
            {
                /* Disable read FIFOS (needed with DMA) */
                tmp = sdmmc_regs->clock_prediv;
                tmp = tmp & ~MMC_PREDIV_APB_RD_EN;
                sdmmc_regs->clock_prediv = tmp;
            }

            sdmmccfgptr->lstat = MMC_WAITING_FOR_RESPONSE;
            sdmmccfgptr->state_step++;

            /* Setup block count */
            sdmmc_command(sdmmccfgptr, SDMMC_SET_BLEN,
                (sdmmccfgptr->bsize << 9));
            sdmmc_regs->block_count = sdmmccfgptr->bsize;

            /* Save sectors to transfer count down value */
            sdmmccfgptr->to_transfer = sdmmccfgptr->bsize;

            /* Enable the response interrupt */
            sdmmc_regs->int_enable = (MMC_INT_END_CMD_RESPONSE |
                MMC_INT_PROGRAM_DONE);

            break;
 
        case 1:
            /* Step 1: If the response was good, then start the
               interrupt */
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Just throw away response */
                sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);
                sdmmccfgptr->state_step++;

                /* Start data write request */
                if (sdmmccfgptr->bsize > 1)
                {
                    /* Issue multi-block write command - this will only
                       work with DMA and will not get a data response
                       interrupt after the transfer. It is the
                       responsibility of the DMA transfer function to
                       check the data transfer flag and data CRC flag
                       and re-issue the request if needed! */
                    sdmmc_command(sdmmccfgptr, SDMMC_WRITE_MULTIPLE,
                        (sdmmccfgptr->sector_start << 9));

                    /* Set states for DMA */
                    sdmmccfgptr->lstat = MMC_DMA;
                    sdmmccfgptr->state = SDMMC_DMA;
                }
                else
                {
                    /* Issue single block write command */
                    sdmmc_command(sdmmccfgptr, SDMMC_WRITE_SINGLE,
                        (sdmmccfgptr->sector_start << 9));
                }
            }
            else
            {
                /* No response, deselect card and set exit state */
                sdmmccfgptr->state_step = 10;
                sdmmc_command(sdmmccfgptr, SDMMC_SELECT_CARD, 0);
            }

            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            break;

        case 2:
            /* Step 2: Start data write cycle */

            /* For a data transfer, the response is returned when the
               dta is ready to be transferred */
            if ((sdmmc_regs->status &
                MMC_STATUS_END_COMMAND_RESPONSE) != 0)
            {
                if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) != 0)
                {
                    /* Response timed out, something is wrong, stop
                       here */
                    sdmmc_regs->int_enable = 0;
                    sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
                    sdmmccfgptr->state = SDMMC_READY;
                    finished = TRUE;
                }
                else
                {
                    /* Just throw away response */
                    sdmmc_get_response(sdmmc_regs, &resp_r1[0], 6);

                    /* If DMA mode, set the finished flag now */
                    if (sdmmccfgptr->state == SDMMC_DMA)
                    {
                        /* Do nothing, this is DMA */
                    }
                    else
                    {
                        /* Enable the program interrupt and fill the
                           FIFO with data to send to the card */
                        buffin = sdmmccfgptr->buffer;
                        for (idx = 0; idx < (SDMMC_BLK_SIZE / 2); idx++)
                        {
                            sdmmc_regs->data_fifo = *buffin;
                            buffin++;
                        }

                        /* Next state */
                        sdmmccfgptr->state_step++;

                        /* Restart the clock */
                        sdmmc_regs->int_clear = MMC_INT_PROGRAM_DONE;
                        sdmmc_clock_start(sdmmc_regs);
                    }
                }
            }
            else
            {
                /* No response, deselect card and set exit state */
                sdmmccfgptr->state_step = 10;
                sdmmc_command(sdmmccfgptr, SDMMC_SELECT_CARD, 0);
                sdmmccfgptr->state = SDMMC_READY;
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
            }

            /* Clear the command response interrupt */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            break;

        case 3:
            /* Step 3: Verify data write (non-DMA only) */

            /* Transfer is done, was there a write error? */
            if ((sdmmc_regs->status &
                MMC_STATUS_CRC_ERROR_WRITE) != 0)
            {
                sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
            }
            else
            {
                sdmmccfgptr->lstat = MMC_OPERATION_OK;
            }

            /* Sequence is complete */
            sdmmc_regs->int_enable = 0;
            sdmmc_regs->int_clear = (MMC_INT_PROGRAM_DONE |
                MMC_INT_DATA_TRANSFER_DONE);
            sdmmccfgptr->state = SDMMC_READY;
            finished = TRUE;
            break;

        default:
            /* There should be no other cases */
            break;
    }

    return finished;
}

/***********************************************************************
 *
 * Function: sdmmc_state_machine
 *
 * Purpose: SDMMC state machine handler
 *
 * Processing:
 *     This function handles the states for the various steps of the
 *     SD/MMC commands and responses. Although some of these can be
 *     done using polling, polling may drag down a system waiting for
 *     the response to come back.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     This function handles the states for the various steps of the
 *     SD/MMC commands and responses. Although some of these can be
 *     done using polling, polling may drag down a system waiting for
 *     the response to come back. Commands that require responses or
 *     data will never wait for the response and instead use interrupts
 *     to handle the data or response. In this type of system, care
 *     should be taken to make sure operations are atomic at a higher
 *     driver level (or the appropriate OS-dependent code can be added
 *     here).
 *
 **********************************************************************/
void sdmmc_state_machine(SDMMC_CFG_T *sdmmccfgptr)
{
    volatile UNS_16 tmp;
    BOOL_32 finished = FALSE;
    SDMMC_REGS_T *sdmmc_regs = sdmmccfgptr->regptr;

    /* Handle the specific major state */
    switch (sdmmccfgptr->state)
    {
        case SDMMC_RESET:
            /* Disable interrupts */
            sdmmc_regs->int_enable = 0;

            /* Empty the data FIFO */
            while ((sdmmc_regs->status & MMC_STATUS_FIFO_EMPTY) == 0)
            {
                tmp = sdmmc_regs->data_fifo;
            }

            /* Set state to ok */
            sdmmccfgptr->lstat = MMC_OPERATION_OK;

            /* Set currrent state to no card */
            sdmmccfgptr->state = SDMMC_NO_CARD;

            /* Issue the initialize all command */
            sdmmc_command(sdmmccfgptr, SDMMC_IDLE, 0);

            /* Do not set finished for this command, as it will shut
               down the clock - this function does leave the clock
               running - there is no callback for this function */
            break;

        case SDMMC_INIT_SD_STATE:
            /* Call the SD initialization state manager */
            finished = sd_init_state_manager(sdmmccfgptr);
            break;

        case SDMMC_INIT_MMC_STATE:
            /* Call the MMC initialization state manager */
            finished = mmc_init_state_manager(sdmmccfgptr);
            break;

        case SDMMC_REG_FETCH:
            /* Only one state in the register fetch function */
            sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
            if ((sdmmc_regs->status & MMC_STATUS_RESPONSE_TO) == 0)
            {
                /* Copy response to buffer */
                sdmmc_get_response(sdmmc_regs, &sdmmccfgptr->buffer[0],
                    sdmmccfgptr->bsize);

                /* No response, stop here with an error */
                sdmmccfgptr->lstat = MMC_OPERATION_OK;
            }
            else
            {
                /* No response, stop here with an error */
                sdmmccfgptr->lstat = MMC_NO_RESPONSE;
            }

            /* Leave the clock and interrupts disabled */
            sdmmccfgptr->state = SDMMC_READY;
            sdmmc_regs->int_enable = 0;
            sdmmc_clock_stop(sdmmccfgptr->regptr);
            finished = TRUE;
            break;

        case SDMMC_READ_STATE:
            /* Call the read request state manager */
            finished = sdmmc_read_request_manager(sdmmccfgptr);
            break;

        case SDMMC_WRITE_STATE:
            /* Call the write request state manager */
            finished = sdmmc_write_request_manager(sdmmccfgptr);
            break;

        case SDMMC_DESELECT:
            if (sdmmccfgptr->state_step == 0)
            {
                sdmmccfgptr->lstat = MMC_WAITING_FOR_RESPONSE;

                /* Next state on next interrupt when clock stops */
                sdmmccfgptr->state_step++;

                /* Issue the deselect command */
                sdmmc_command(sdmmccfgptr, SDMMC_SELECT_CARD, 0);

                /* Enable the response interrupt, although this command
                   doesn't return a response, the interrupt will still
                   fire at the command completion */
                sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
                sdmmc_regs->int_enable = MMC_INT_END_CMD_RESPONSE;
            }
            else if (sdmmccfgptr->state_step == 1)
            {
                /* Disable interrupts */
                sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
                sdmmccfgptr->regptr->int_enable = 0;

                /* Set state to ok */
                sdmmccfgptr->lstat = MMC_OPERATION_OK;

                /* Set currrent state to ready */
                sdmmccfgptr->state = SDMMC_READY;

                finished = TRUE;
            }
            break;

        case SDMMC_EJECTING:
            if (sdmmccfgptr->state_step == 0)
            {
                sdmmccfgptr->lstat = MMC_WAITING_FOR_RESPONSE;

                /* Next state on next interrupt when clock stops */
                sdmmccfgptr->state_step++;

                /* Issue the deactivate command */
                sdmmc_command(sdmmccfgptr, SDMMC_INACTIVE,
                    (UNS_32) (sdmmccfgptr->rca << 16));

                /* Enable the response interrupt, although this command
                   doesn't return a response, the interrupt will still
                   fire at the command completion */
                sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
                sdmmc_regs->int_enable = MMC_INT_END_CMD_RESPONSE;
            }
            else if (sdmmccfgptr->state_step == 1)
            {
                /* Disable interrupts */
                sdmmc_regs->int_clear = MMC_INT_END_CMD_RESPONSE;
                sdmmccfgptr->regptr->int_enable = 0;

                /* Set state to ok */
                sdmmccfgptr->lstat = MMC_OPERATION_OK;

                /* Set currrent state to no card */
                sdmmccfgptr->state = SDMMC_NO_CARD;

                finished = TRUE;
            }
            break;

        case SDMMC_NO_CARD:
        case SDMMC_READY:
        default:
            /* These cases should never happen, clear all the interrupts
               to be safe */
            sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
            sdmmc_regs->int_clear = SDMMC_ALL_INTS;
            finished = TRUE;
            break;
    }

    /* If the last step was complete in the state sequence and a
       callback exists, then call it to signal completion of the
       requested SDMMC function */
    if (finished == TRUE)
    {
        sdmmc_clock_stop(sdmmc_regs);

        if (sdmmccfg.sdmmc_cb != NULL)
        {
            sdmmccfg.sdmmc_cb();
        }
    }
}

/***********************************************************************
 *
 * Function: sdmmc_reset_card
 *
 * Purpose: Reset all SD or MMC cards
 *
 * Processing:
 *     Set the current driver state to SDMMC_RESET and call the
 *     sdmmc_state_machine() fucntion. Return _NO_ERROR to the caller.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: Always returns _NO_ERROR
 *
 * Notes: A reset can be performed anytime and without a card installed
 *
 **********************************************************************/
INT_32 sdmmc_reset_card(SDMMC_CFG_T *sdmmccfgptr)
{
    /* Switch to reset state and start the state engine */
    sdmmccfgptr->state = SDMMC_RESET;
    sdmmc_state_machine(sdmmccfgptr);

    return _NO_ERROR;
}

/***********************************************************************
 *
 * Function: sdmmc_start_ocr_run
 *
 * Purpose: Start the SDMMC OCR initialization sequence
 *
 * Processing:
 *     If the current state is not SDMMC_NO_CARD, set the operation
 *     status to failed and return _ERROR to the caller. Otherwise,
 *     save a copy of the OCR init value and set the SDMMC clock to
 *     the initialization clock rate. Set the driver state to
 *     SDMMC_INIT_SD_STATE and the the current driver step to 0. Set
 *     the default block length to SDMMC_BLK_SIZE. Call the state
 *     manager function (sdmmc_state_machine()) and return _NO_ERROR
 *     to the caller.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: _ERROR if a card was already detect, otherwise _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_start_ocr_run(SDMMC_CFG_T *sdmmccfgptr,
                           UNS_32 ocrval)
{
    INT_32 status = _ERROR;
    SDMMC_REGS_T *sdmmc_regs = sdmmccfgptr->regptr;

    /* Only continue if the current state is "no card" */
    if (sdmmccfgptr->state == SDMMC_NO_CARD)
    {
        /* Save OCR check value to use */
        sdmmccfgptr->ocrval = (UNS_32) ocrval;

        /* Set initial MMC clock rate to 390KHz for the OCR sequence */
        sdmmc_set_clock(sdmmc_regs, SDMMC_OCR_CLOCK);

        /* Set the state to SD init and the minor step # to 0 */
        sdmmccfgptr->state = SDMMC_INIT_SD_STATE;
        sdmmccfgptr->state_step = 0;

        /* Assume this is a not a SD card for now */
        sdmmccfgptr->sd_card = 0;

        /* Set default block size for all transfers */
        sdmmc_regs->block_len = SDMMC_BLK_SIZE;

        /* Call the state manager to start the processing sequence */
        sdmmc_state_machine(sdmmccfgptr);

        /* At this point, the state engine takes over - the driver
           can be occasionally polled to determine when the operation
           is complete, or can use the callback function to signal
           when it is complete */
        status = _NO_ERROR;
    }
    else
    {
        /* Trying to perform a detect and initialization sequence in
           another state is not possible. Either the card needs to be
           reset first, or the initialization shouldn't of happened
           at all */

        /* Disable interrupts and set status to failed */
        sdmmc_regs->int_enable = 0;
        sdmmccfgptr->lstat = MMC_OPERATION_FAILED;
    }

    return status;
}

/***********************************************************************
 *
 * Function: sdmmc_eject_card
 *
 * Purpose: Eject the installed card
 *
 * Processing:
 *     If the current driver state is not SDMMC_READY, return _ERROR
 *     to the caller. Otherwise, Set the driver state to SDMMC_EJECTING
 *     and the state step to 0. Call the state manager function
 *     (sdmmc_state_machine()) and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: _ERROR if the card can't be ejected, otherwise _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_eject_card(SDMMC_CFG_T *sdmmccfgptr)
{
    INT_32 status = _ERROR;

    /* The eject command only works with a card that is ready */
    if (sdmmccfgptr->state == SDMMC_READY)
    {
        /* Switch to ejecting state */
        sdmmccfgptr->state = SDMMC_EJECTING;
        sdmmccfgptr->state_step = 0;
        status = _NO_ERROR;
        sdmmc_state_machine(sdmmccfgptr);
    }

    return status;
}

/***********************************************************************
 *
 * Function:                     
 *
 * Purpose: Requests a register from the SDMMC card
 *
 * Processing:
 *     If the current driver state is not SDMMC_READY, return _ERROR
 *     to the caller. Otherwise, Set the driver state to SDMMC_REG_FETCH
 *     and the state step to 0. Save the location of the buffer and the
 *     the expected response size. Issue the request command, enable
 *     the response interrupt, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *     cmd         : SDMMC command request
 *
 * Outputs: None
 *
 * Returns: _ERROR if the card isn't ready, otherwise _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_get_reg(SDMMC_CFG_T *sdmmccfgptr,
                     UNS_32 cmd)
{
    INT_32 status = _ERROR;

    /* The get CID command only works with a card that is ready */
    if (sdmmccfgptr->state == SDMMC_READY)
    {
        /* Switch to register fetch state and set up the state
           controller */
        sdmmccfgptr->state = SDMMC_REG_FETCH;
        sdmmccfgptr->state_step = 0;

        /* Issue the command to the controller now */
        sdmmc_command(sdmmccfgptr, cmd, (sdmmccfgptr->rca << 16));

        /* Enable response interrupt */
        sdmmccfgptr->regptr->int_enable = MMC_INT_END_CMD_RESPONSE;

        status = _NO_ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * Function: sdmmc_rw_request
 *
 * Purpose: Start a sector read/write request sequence
 *
 * Processing:
 *     If the current driver state is not SDMMC_READY, return _ERROR
 *     to the caller. Otherwise, Set the driver state to the passed
 *     state and the state step to 0. Call the state manager function
 *     (sdmmc_state_machine()) and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *     rw_state   : Read or write state value
 *
 * Outputs: None
 *
 * Returns: _ERROR if the card isn't ready, otherwise _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_rw_request(SDMMC_CFG_T *sdmmccfgptr,
                        SDMMC_MAJOR_STATE_T rw_state)
{
    INT_32 status = _ERROR;

    /* The eject command only works with a card that is ready */
    if (sdmmccfgptr->state == SDMMC_READY)
    {
        /* multi-sector reads are only allowed with DMA */
        if ((sdmmccfgptr->use_dma == FALSE) &&
            (sdmmccfgptr->bsize > 1))
        {
            /* Driver cannot handle multi-sector without DMA, exit
               with an error */
            status = _ERROR;
        }
        else
        {
            /* Switch to the passed state */
            sdmmccfgptr->state = rw_state;
            sdmmccfgptr->state_step = 0;

            /* Start the transfer by calling the state controller */
            sdmmc_state_machine(sdmmccfgptr);

            /* For now, no errors */
            status = _NO_ERROR;
        }
    }

    return status;
}

/***********************************************************************
 *
 * Function: sdmmc_write_request
 *
 * Purpose: Start a sector write request sequence
 *
 * Processing:
 *     If the current driver state is not SDMMC_READY, return _ERROR
 *     to the caller. Otherwise, Set the driver state to
 *     SDMMC_WRITE_STATE and the state step to 0. Save the location of
 *     the buffer, number of sectors, first sector to write, and DMA
 *     flag. Call the state manager function (sdmmc_state_machine())
 *     and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     sdmmccfgptr: Pointer to SDMMC config structure
 *     sdmmc_trans: Pointer to a write setup structure
 *
 * Outputs: None
 *
 * Returns: _ERROR if the card isn't ready, otherwise _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_write_request(SDMMC_CFG_T *sdmmccfgptr,
                           SDMMC_SECTORS_T *sdmmc_trans)
{
    INT_32 status = _ERROR;

    /* The eject command only works with a card that is ready */
    if (sdmmccfgptr->state == SDMMC_READY)
    {
        /* Save size and location of buffer */
        sdmmccfgptr->buffer = (UNS_16 *) sdmmc_trans->buffer;

        /* Size is in sectors for read and write */
        sdmmccfgptr->bsize = (INT_32) sdmmc_trans->number_sectors;
        sdmmccfgptr->sector_start =
            (UNS_32) sdmmc_trans->logical_sector;
        sdmmccfgptr->use_dma = sdmmc_trans->use_dma;

        /* multi-sector reads are only allowed with DMA */
        if ((sdmmccfgptr->use_dma == FALSE) &&
            (sdmmccfgptr->bsize > 1))
        {
            /* Driver cannot handle multi-sector without DMA, exit
               with an error */
            status = _ERROR;
        }
        else
        {
            /* Switch to write state and set up the state controller */
            sdmmccfgptr->state = SDMMC_WRITE_STATE;
            sdmmccfgptr->state_step = 0;

            /* Start the transfer by calling the state controller */
            sdmmc_state_machine(sdmmccfgptr);

            /* For now, no errors */
            status = _NO_ERROR;
        }
    }

    return status;
}

/***********************************************************************
 * SDMMC driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: sdmmc_open
 *
 * Purpose: Open the SDMMC controller
 *
 * Processing:
 *     If the driver been previously initialized or the passed base
 *     address isn't the SDMMC base address, return 0 to the caller.
 *     Otherwise, save the register base pointer and disable the
 *     SDMMC controller. Set init to TRUE, the default callback function
 *     to NULL, the default state to no card, and the last state to
 *     OK. Set the default card address address to 0x0111. Enable the
 *     SDMMC controller with the lowest clock speeds. Disable interrupts
 *     and return the address of the SDMMC driver configuration
 *     structure to the caller.
 *
 * Parameters:
 *     ipbase: SDMMC descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a SDMMC config structure or 0
 *
 * Notes:
 *     This function only opens and initializes the controller. Card
 *     detection and initialization occur seperately.
 *
 **********************************************************************/
INT_32 sdmmc_open(void *ipbase,
                  INT_32 arg)
{
    volatile UNS_16 tmp;
    INT_32 status = 0;

    if ((sdmmccfg.init == FALSE) && ((SDMMC_REGS_T *) ipbase == SDMMC))
    {
        /* Save and return address of peripheral block */
        sdmmccfg.regptr = (SDMMC_REGS_T *) ipbase;

        /* Disable SDMMC controller */
        sdmmccfg.regptr->clock_prediv = 0;

        /* Initial callback function is NULL */
        sdmmccfg.sdmmc_cb = NULL;

        /* Device is valid and not previously initialized */
        sdmmccfg.init = TRUE;

        /* Set the state to "no card" and the status to ok */
        sdmmccfg.state = SDMMC_NO_CARD;
        sdmmccfg.lstat = MMC_OPERATION_OK;

        /* For now, only one card is supported, so only one relative
           card address is needed, any number that is not 0x0000 will
           do ok */
        sdmmccfg.rca = 0x0111;

        /* Enable SDMMC controller and keep clock rate very low to
           save power */
        sdmmccfg.regptr->clock_prediv = (MMC_PREDIV_APB_RD_EN |
            MMC_PREDIV_EN | MMC_PREDIV(8));
        sdmmccfg.regptr->clock_rate = MMC_CLOCK_DIV64;

        /* Initially mask all interrupts */
        sdmmccfg.regptr->int_enable = 0;

        /* Pre-empty the data FIFO */
        while ((sdmmccfg.regptr->status &
            MMC_STATUS_FIFO_EMPTY) == 0)
        {
            tmp = sdmmccfg.regptr->data_fifo;
        }

        /* Return pointer to the SDMMC data structure */
        status = (INT_32) &sdmmccfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: sdmmc_close
 *
 * Purpose: Close the SDMMC
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable all SDMMC
 *     interrupts, disable the SDMMC controller, set init to FALSE,
 *     and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to SDMMC config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS sdmmc_close(INT_32 devid)
{
    SDMMC_CFG_T *sdmmccfgptr = (SDMMC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (sdmmccfgptr->init == TRUE)
    {
        /* Mask all interrupts, then disable the controller */
        sdmmccfgptr->regptr->int_enable = 0;
        sdmmccfgptr->regptr->clock_prediv = MMC_PREDIV(8);

        sdmmccfgptr->init = FALSE;
        status = _NO_ERROR;
    }

    return status;
}

/***********************************************************************
 *
 * Function: sdmmc_ioctl
 *
 * Purpose: SDMMC configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate SDMMC parameter.
 *
 * Parameters:
 *     devid: Pointer to SDMMC config structure
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
STATUS sdmmc_ioctl(INT_32 devid,
                   INT_32 cmd,
                   INT_32 arg)
{
    UNS_32 tmp;
    UNS_16 *buffer;
    INT_32 idx;
    SDMMC_SECTORS_T *sdmmc_trans;
    SDMMC_REGS_T *sdmmc_regs;
    SDMMC_CFG_T *sdmmccfgptr = (SDMMC_CFG_T *) devid;
    STATUS status = _ERROR;

    if (sdmmccfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        sdmmc_regs = sdmmccfgptr->regptr;

        switch (cmd)
        {
            case MMC_SET_CLOCK:
                /* Set new SDMMC clock speed */
                sdmmc_set_clock(sdmmccfgptr->regptr, (UNS_32) arg);
                break;

            case MMC_ENABLE_INTS:
                /* Enable selected SDMMC interrupts */
                tmp = (UNS_32) arg & SDMMC_ALL_INTS;
                sdmmc_regs->int_enable = sdmmc_regs->int_enable | tmp;
                break;

            case MMC_DISABLE_INTS:
                /* Disable selected SDMMC interrupts */
                tmp = (UNS_32) arg & SDMMC_ALL_INTS;
                sdmmc_regs->int_enable = sdmmc_regs->int_enable & ~tmp;
                break;

            case MMC_CLEAR_INTS:
                /* Clear selected SDMMC interrupts */
                tmp = (UNS_32) arg & SDMMC_ALL_INTS;
                sdmmc_regs->int_clear = tmp;
                break;

            case MMC_INSTALL_CB:
                /* Install interrupt callback function */
                sdmmccfgptr->sdmmc_cb = (PFV) arg;
                break;

            case MMC_SD_CHECK:
                /* Returns 1 if the card is a SD card, 0 if it is an
                   MMC card */
                status = sdmmccfgptr->sd_card;
                break;

            case MMC_GET_OCR:
                /* Copy the saved OCR value to the passed address */
                if (sdmmccfgptr->state == SDMMC_NO_CARD)
                {
                    /* If there is no card, these is no OCR */
                    status = _ERROR;
                }
                else
                {
                    buffer = (UNS_16 *) arg;
                    for (idx = 0; idx < (SDMMC_OCR_SIZE / 2); idx++)
                    {
                        buffer[idx] = sdmmccfgptr->saved_ocr[idx];
                    }
                }
                break;

            case MMC_RESET:
                /* Reset the SDMMC card - initialization will need to
                   be re-performed after this */
                status = sdmmc_reset_card(sdmmccfgptr);
                break;

            case MMC_DETECT:
                /* Detects an SD or MMC card */
                status = sdmmc_start_ocr_run(sdmmccfgptr, (UNS_32) arg);
                break;

            case MMC_CARD_EJECT:
                /* Ejects the selected card */
                status = sdmmc_eject_card(sdmmccfgptr);
                break;
 
            case MMC_GET_CID:
                /* Requests the card identification (CID) register */
                sdmmccfgptr->buffer = (UNS_16 *) arg;
                sdmmccfgptr->bsize = SDMMC_CID_SIZE;
                status = sdmmc_get_reg(sdmmccfgptr, SDMMC_SEND_CID);
                break;

            case MMC_GET_CSD:
                /* Requests the card specific data (CSD) register */
                sdmmccfgptr->buffer = (UNS_16 *) arg;
                sdmmccfgptr->bsize = SDMMC_CSD_SIZE;
                status = sdmmc_get_reg(sdmmccfgptr, SDMMC_SEND_CSD);
                break;

            case MMC_READ_REQUEST:
                /* Start state handler for a read operation */
                sdmmc_trans = (SDMMC_SECTORS_T *) arg;
                sdmmccfgptr->buffer = (UNS_16 *) sdmmc_trans->buffer;
                sdmmccfgptr->bsize = (INT_32)
                    sdmmc_trans->number_sectors;
                sdmmccfgptr->sector_start =
                    (UNS_32) sdmmc_trans->logical_sector;
                sdmmccfgptr->use_dma = sdmmc_trans->use_dma;
                status = sdmmc_rw_request(sdmmccfgptr,
                    SDMMC_READ_STATE);
                break;

            case MMC_WRITE_REQUEST:
                /* Start state handler for a write operation */
                sdmmc_trans = (SDMMC_SECTORS_T *) arg;
                sdmmccfgptr->buffer = (UNS_16 *) sdmmc_trans->buffer;
                sdmmccfgptr->bsize = (INT_32)
                    sdmmc_trans->number_sectors;
                sdmmccfgptr->sector_start =
                    (UNS_32) sdmmc_trans->logical_sector;
                sdmmccfgptr->use_dma = sdmmc_trans->use_dma;
                status = sdmmc_rw_request(sdmmccfgptr,
                    SDMMC_WRITE_STATE);
                break;

            case MMC_DESELECT_CARD:
                /* Deselect the active SDMMC card */
                sdmmccfgptr->state = SDMMC_DESELECT;
                sdmmccfgptr->state_step = 0;
                sdmmc_state_machine(sdmmccfgptr);
                break;

            case MMC_GET_STATUS:
                /* Return a MMC interface status */
                switch (arg)
                {
                    case MMC_DRV_STATUS:
                        /* Returns the status of the current
                           operation */
                        status = sdmmccfgptr->lstat;
                        break;

                    case MMC_INTS_ENABLED:
                        /* Returns enabled interrupts */
                        status = (INT_32) (sdmmc_regs->int_enable &
                            SDMMC_ALL_INTS);
                        break;

                    case MMC_INTS_PENDING:
                        /* Returns pending interrupts */
                        status = (INT_32) sdmmc_regs->int_status;
                        break;

                    case MMC_GET_SECT_SIZE:
                        /* Return sector size */
                        status = SDMMC_BLK_SIZE;
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
 * Function: sdmmc_read
 *
 * Purpose: Read a block of data
 *
 * Processing:
 *     Stub function only, does nothing
 *
 * Parameters:
 *     devid:     Pointer to SDMMC config structure
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read (should be SDMMC_BLK_SIZE)
 *
 * Outputs: None
 *
 * Returns: Always returns 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_read(INT_32 devid,
                  void *buffer,
                  INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: sdmmc_write
 *
 * Purpose: Write a block of data
 *
 * Processing:
 *     Stub function only, does nothing
 *
 * Parameters:
 *     devid:   Pointer to SDMMC config structure
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Always returns 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 mmc_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: sdmmc_isr
 *
 * Purpose: SDMMC interrupt handler
 *
 * Processing:
 *     Routes all functions to the SDMMC state handler and clears any
 *     pending interrupts.
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
void sdmmc_isr(void)
{
    /* Perform state logic */
    sdmmc_state_machine(&sdmmccfg);
}
