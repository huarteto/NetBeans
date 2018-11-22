/***********************************************************************
 * $Id:: lpc313x_dma_driver.c 1180 2008-09-11 19:32:40Z pdurgesh       $
 *
 * Project: LPC313x DMA driver
 *
 * Description:
 *     This file contains driver support for the LPC313x DMA controller.
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
***********************************************************************/

#include "lpc313x_dma_driver.h"
#include "lpc313x_intc_driver.h"
#include "lpc313x_cgu_driver.h"
#include "lpc_arm922t_cp15_driver.h"

/***********************************************************************
 * DMA driver private data
***********************************************************************/

/* DMA driver control structure */
typedef struct
{
  BOOL_32 init;
  INT_32  alloc_ch [DMA_MAX_CHANNELS];
  PF_DMACB  cb [DMA_MAX_CHANNELS];
  PF_DMACB	soft_cb;
  INT_32  num_alloc_ch;    /* Number of allocated channels */
  DMAC_REGS_T *pdma;
} DMA_DRV_DATA_T;

/* DMAS driver data */
static DMA_DRV_DATA_T g_dmadrv_dat;

/***********************************************************************
 * DMA driver private functions
***********************************************************************/

/***********************************************************************
 *
 * Function: dma_interrupt
 *
 * Purpose: DMA controller interrupt handler
 *
 * Processing:
 *     This function is called when a DMA interrupt occurs. It looks at
 *     the DMA statuses and calls the user defined callback function
 *     for the active DMA channel if it exists. If a callback function
 *     doesn't exist, then interrupt support for the DMA channel is
 *     disabled.
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
void dma_interrupt(void)
{
  INT_32 ch;
  UNS_32 sts_int;

  /* Get DMA statuses */
  sts_int = g_dmadrv_dat.pdma->irq_status_clear;

  /* check for global ints */
  if (sts_int & DMA_IRQ_SOFT)
  {
    /* check callback is installed */
    if (g_dmadrv_dat.soft_cb != NULL)
    {
      g_dmadrv_dat.soft_cb(g_dmadrv_dat.pdma);
    }
    else
    {
      /* Interrupt is pending, but no user callback function
         exists, so disable the interruptsto prevent the interrupt
      from continuously firing */
      g_dmadrv_dat.pdma->irq_mask |= DMA_IRQ_SOFT;
      g_dmadrv_dat.pdma->irq_status_clear = DMA_IRQ_SOFT;
    }
  }

  /* TBD: How to handle AHB abort interrupt */

  /* Check all pending interrupt statuses */
  ch = 0;
  while (ch < DMA_MAX_CHANNELS)
  {
    /* Check channel interrupt status */
    if ((sts_int & DMA_IRQ_CH(ch)) != 0)
    {
      /* Channel interrupt is pending */
      if (g_dmadrv_dat.cb [ch] != NULL)
      {
        /* Call user defined callback function */
        g_dmadrv_dat.cb [ch](g_dmadrv_dat.pdma);
      }
      else
      {
        /* Interrupt is pending, but no user callback function
           exists, so disable the interrupts for this channel
           to prevent the interrupt from continuously firing */
        g_dmadrv_dat.pdma->irq_mask |= DMA_IRQ_CH(ch);
        g_dmadrv_dat.pdma->irq_status_clear = DMA_IRQ_CH(ch);
      }
    }

    /* Next channel */
    ch++;
  }
}

/***********************************************************************
 * DMA driver public functions
***********************************************************************/

/***********************************************************************
 *
 * Function: dma_init
 *
 * Purpose: Initial DMA controller and driver
 *
 * Processing:
 *     This function sets up the DMA controller as initially disabled.
 *     All DMA channels used by the driver are unallocated.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns:
 *     _ERROR if the device was already initialized, otherside _NO_ERROR
 *
 * Notes: None
 *
 **********************************************************************/
STATUS dma_init(void)
{
  INT_32 idx;
  STATUS init = _ERROR;

  /* Only continue if driver has not been previously initialized */
  if (g_dmadrv_dat.init == FALSE)
  {
    g_dmadrv_dat.init = TRUE;
    g_dmadrv_dat.num_alloc_ch = 0;

    /* Save base address of DMA controller registers */
    g_dmadrv_dat.pdma = (DMAC_REGS_T *) DMA_BASE;

    /* Enable clock to DMA controller (for now) */
    cgu_clk_en_dis(CGU_SB_DMA_PCLK_ID, 1);
    cgu_clk_en_dis(CGU_SB_DMA_CLK_GATED_ID, 1);

    /* Make sure DMA controller and all channels are disabled. */
    g_dmadrv_dat.pdma->alt_enable = 0;
    g_dmadrv_dat.pdma->irq_mask = DMA_IRQ_ALL_MASK;

    /* Clear interrupt and error statuses */
    g_dmadrv_dat.pdma->irq_status_clear = DMA_IRQ_ALL_MASK;

    /* All DMA channels are initially disabled and unallocated */
    for (idx = 0; idx < DMA_MAX_CHANNELS; idx++)
    {
      /* Channel is currently unallocated */
      g_dmadrv_dat.alloc_ch [idx] = FALSE;
      g_dmadrv_dat.cb [idx] = NULL;

      /* Make sure channel is disabled */
      g_dmadrv_dat.pdma->channel [idx].enable = 0;
      g_dmadrv_dat.pdma->channel [idx].config = 0;
    }
    g_dmadrv_dat.soft_cb = NULL;

    init = _NO_ERROR;
  }

  return init;
}

/***********************************************************************
 *
 * Function: dma_alloc_channel
 *
 * Purpose: Allocate a channel for DMA
 *
 * Processing:
 *     If the passed channel is (-1), then a search loop is used to
 *     find the first unallocated channel. The channel value is saved
 *     and then checked to make sure it is unallocated. If it is
 *     already allocated or not allocatable, then an error is return to
 *     the caller. If the channel is not allocated, the channel is
 *     marked as allocated and the channel ID is returned to the caller.
 *     If at least one channel is active, the DMA clock is enabled.
 *
 * Parameters:
 *     ch : Must be 0 (highest priority) to 11, or -1 for auto-allocation
 *     cb : Pointer to user callback function when an interrupt occurs
 *
 * Outputs: None
 *
 * Returns: The channel index, or _ERROR if a channel wasn't allocated
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 dma_alloc_channel(INT_32 ch, PF_DMACB cb)
{
  INT_32 challoc = ch;

  /* If auto-allocate is used, find the first available channel
     starting with the highest priority first */
  if (ch == -1)
  {
    ch = 0;
    challoc = _ERROR;
    while (ch < DMA_MAX_CHANNELS)
    {
      if (g_dmadrv_dat.alloc_ch [ch] == FALSE)
      {
        /* Channel is free, use it */
        challoc = ch;
        ch = DMA_MAX_CHANNELS;
      }
      else
      {
        /* Try next channel */
        ch++;
      }
    }
  }

  /* Only continue if channel is ok */
  if (challoc != _ERROR)
  {
    /* If the current channel is allocated already, then return an
        error instead */
    if (g_dmadrv_dat.alloc_ch [challoc] == FALSE)
    {
      /* Channel is free, so use it */
      g_dmadrv_dat.alloc_ch [challoc] = TRUE;
      g_dmadrv_dat.cb [challoc] = cb;
      g_dmadrv_dat.num_alloc_ch++;

      /* Enable DMA clock if at least 1 DMA channel is used */
      if (g_dmadrv_dat.num_alloc_ch == 1)
      {
        /* Install DMA interrupt handler in interrupt controller
           and enable DMA interrupt */
        int_install_irq_handler(IRQ_DMA, (PFV) dma_interrupt);
        int_enable(IRQ_DMA);
      }
    }
    else
    {
      /* Selected channel is allocated, return an error */
      challoc = _ERROR;
    }
  }

  return challoc;
}

/***********************************************************************
 *
 * Function: dma_free_channel
 *
 * Purpose: Return (free) an allocated DMA channel
 *
 * Processing:
 *     If the channel has been previously allocated, then deallocate
 *     the channel and disable the channel in the DMA controller. If
 *     no other DMA channels are enabled, the disable the DMA controller
 *     along with the controller clock and DMA interrupts.
 *
 * Parameters:
 *     ch : Must be 0 to 11
 *
 * Outputs: None
 *
 * Returns: _NO_ERROR if the channel was freed, otherwise _ERROR
 *
 * Notes: None
 *
 **********************************************************************/
STATUS dma_free_channel(INT_32 ch)
{
  STATUS status = _ERROR;
  if (g_dmadrv_dat.alloc_ch [ch] == TRUE)
  {
    /* Deallocate channel */
    g_dmadrv_dat.alloc_ch [ch] = FALSE;
    g_dmadrv_dat.num_alloc_ch--;

    /* Shut down channel */
    g_dmadrv_dat.pdma->channel [ch].enable = 0;
    g_dmadrv_dat.pdma->channel [ch].config = 0;

    /* If no other DMA channels are enabled, then disable the DMA
       controller and disable the DMA clock */
    if (g_dmadrv_dat.num_alloc_ch == 0)
    {
      /* Disable DMA interrupt */
      int_install_irq_handler(IRQ_DMA, (PFV) NULL);
      int_disable(IRQ_DMA);
    }

    status = _NO_ERROR;
  }

  return status;
}


/***********************************************************************
 *
 * Function: dma_get_base
 *
 * Purpose: Return pointer to DMA registers
 *
 * Processing:
 *     Convert the source and destination addresses to physical
 *     addresses, Call dma_setup_link_phy() to place the source,
 *     destination, and DMA control word entries into the passed
 *     structure.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Pointer to DMA base registers
 *
 * Notes: None
 *
 **********************************************************************/
DMAC_REGS_T *dma_get_base(void)
{
  return g_dmadrv_dat.pdma;
}
