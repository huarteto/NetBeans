/***********************************************************************
 * $Id:: lh79524_dma_driver.c 27 2007-08-31 19:35:06Z kevinw           $
 *
 * Project: LH79520 DMA driver
 *
 * Description:
 *     This file contains driver support for the DMA module on the
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

#include "lh79524_iocon.h"
#include "lh79524_rcpc.h" 
#include "lh79524_dma_driver.h"

/***********************************************************************
 * DMA driver private data
 **********************************************************************/

/* DMA device configuration structure */
STATIC DMA_CFG_T dmacfg;

/* DMA external channel, 0 for DREQ0, 1 for DREQ1 */
INT_32 dma_ext_channel = 0; 
/***********************************************************************
 * DMA driver public functions
 **********************************************************************/
/***********************************************************************
 *
 * Function: dma_open
 *
 * Purpose: Open the DMA controller
 *
 * Processing:
 *     If init is not FALSE, return 0x00000000 to the caller. Otherwise,
 *      return a pointer to the DMA config structure to the caller.
 *
 * Parameters:
 *     ipbase: DMA descriptor device address
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a DMA config structure or 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 dma_open(void *ipbase, INT_32 arg)
{
    INT_32 status = 0;

    if (dmacfg.init == FALSE)
    {
        /* Device is valid and not previously initialized */
        dmacfg.init = TRUE;

        /* Save and return address of peripheral block */
        dmacfg.regptr = (DMAC_REGS_T *) ipbase;

        /* Enable DMA clock in RCPC */
        RCPC->ahbclkctrl &= ~RCPC_AHBCLKCTRL_DMA_DISABLE;
        
        /* Default state of DMA is disabled */
        dmacfg.regptr->stream0.ctrl = 0x0;
        dmacfg.regptr->stream1.ctrl = 0x0;
        dmacfg.regptr->stream2.ctrl = 0x0;
        dmacfg.regptr->stream3.ctrl = 0x0;
        
        /* Return pointer to DMA configuration structure */
        status = (INT_32) &dmacfg;
    }

    return status;
}

/***********************************************************************
 *
 * Function: dma_close
 *
 * Purpose: Close the DMA controller
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, check the status of
 *     the freeze and enable bits. If they are set, then return
 *     SMA_CANT_STOP to the caller as the device can no longer be
 *     stopped. Otherwise, disable the DMA, set init to
 *     FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to DMA config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS dma_close(INT_32 devid)
{
    DMA_CFG_T *dmacfgptr = (DMA_CFG_T *) devid;
    STATUS status = _ERROR;

    if (dmacfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        dmacfgptr->init = FALSE;
        
        /* Disable DMA controller */
        dmacfg.regptr->stream0.ctrl = 0;
        dmacfg.regptr->stream0.sourcelo = 0;
        dmacfg.regptr->stream0.sourcehi = 0;
        dmacfg.regptr->stream0.destlo = 0;
        dmacfg.regptr->stream0.desthi = 0;
        dmacfg.regptr->stream0.max = 0;

        dmacfg.regptr->stream1.ctrl = 0;
        dmacfg.regptr->stream1.sourcelo = 0;
        dmacfg.regptr->stream1.sourcehi = 0;
        dmacfg.regptr->stream1.destlo = 0;
        dmacfg.regptr->stream1.desthi = 0;
        dmacfg.regptr->stream1.max = 0;

        dmacfg.regptr->stream2.ctrl = 0;
        dmacfg.regptr->stream2.sourcelo = 0;
        dmacfg.regptr->stream2.sourcehi = 0;
        dmacfg.regptr->stream2.destlo = 0;
        dmacfg.regptr->stream2.desthi = 0;
        dmacfg.regptr->stream2.max = 0;

        dmacfg.regptr->stream3.ctrl = 0;
        dmacfg.regptr->stream3.sourcelo = 0;
        dmacfg.regptr->stream3.sourcehi = 0;
        dmacfg.regptr->stream3.destlo = 0;
        dmacfg.regptr->stream3.desthi = 0;
        dmacfg.regptr->stream3.max = 0;
   
        dmacfg.regptr->mask = 0;
        dmacfg.regptr->clear = DMAC_ALL_FLAGS;
    }

    return status;
}

/***********************************************************************
 *
 * Function: dma_ioctl
 *
 * Purpose: DMA configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate DMA
 *     parameter.
 *
 * Parameters:
 *     devid: Pointer to DMA config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: DMA status -
 *         DMAC_EOT0
 *         DMAC_EOT1
 *         DMAC_EOT2
 *         DMAC_EOT3
 *         DMAC_ERROR0
 *         DMAC_ERROR1
 *         DMAC_ERROR2
 *         DMAC_ERROR3
 *         DMAC_ACTIVE0 
 *         DMAC_ACTIVE1 
 *         DMAC_ACTIVE2 
 *         DMAC_ACTIVE3 
 *
 *
 **********************************************************************/
STATUS dma_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
    DMAC_REGS_T *dma;
    DMA_CFG_T *dmacfgptr = (DMA_CFG_T *) devid;
    STATUS status = _ERROR;
    
    INT_32 temp;

    if (dmacfgptr->init == TRUE)
    {
        status = _NO_ERROR;
        dma = dmacfgptr->regptr;

        switch (cmd)
        {
            /* Clear DMA status, used in DMA interrupt status clear */
            case DMA_CLEAR_STATUS:
                 /* arg is the status mask must be one of followings -
                    DMAC_EOT0
                    DMAC_EOT1
                    DMAC_EOT2
                    DMAC_EOT3
                    DMAC_ERROR0
                    DMAC_ERROR1
                    DMAC_ERROR2
                    DMAC_ERROR3
                    DMAC_ALL_FLAGS
                 */
                 DMAC->clear = arg;
                 break;
                 
            /* Enable the interrupt for DMA */
            case DMA_ENABLE_INT:
                 /* arg is the int mask must be one of followings -
                    DMAC_EOT0
                    DMAC_EOT1
                    DMAC_EOT2
                    DMAC_EOT3
                    DMAC_ERROR0
                    DMAC_ERROR1
                    DMAC_ERROR2
                    DMAC_ERROR3
                    DMAC_ALL_FLAGS
                 */
                 DMAC->mask |= arg;
                 break;
                 
            case DMA_DISABLE_INT:
                 /* arg is the int mask must be one of followings -
                    DMAC_EOT0
                    DMAC_EOT1
                    DMAC_EOT2
                    DMAC_EOT3
                    DMAC_ERROR0
                    DMAC_ERROR1
                    DMAC_ERROR2
                    DMAC_ERROR3
                    DMAC_ALL_FLAGS
                 */
                 DMAC->mask &= ~arg;
                 break;
                 
            /* DMA memory to memory set source address, arg is address */
            case DMA_SET_M2M_SRC:
                 DMAC->stream3.sourcelo = arg & _BITMASK(16);
                 DMAC->stream3.sourcehi = (arg >> 16) & _BITMASK(16);
                 break;
                 
            /* DMA memory to memory set destination address, arg is 
            address */
            case DMA_SET_M2M_DEST:
                 DMAC->stream3.destlo = arg & _BITMASK(16);
                 DMAC->stream3.desthi = (arg >> 16) & _BITMASK(16);
                 break;
                 
            /*   The number of bytes to transfer; nbytes / src_width
            must be less than 65536 and greater than 0 or this
            function will return failure. DMA_SET_M2M_SRC_WIDTH must
            be executed before this function call */
            case DMA_SET_M2M_BYTES:
                 temp = DMAC->stream3.ctrl;
                 temp &= (_BIT(3)|_BIT(4));
                 if (temp == DMAC_CTRL_SOSIZE_1BYTE)
                    temp = 1;
                 else if (temp == DMAC_CTRL_SOSIZE_2BYTE)
                    temp = 2;
                 else
                    temp = 4;   
                 arg /= temp;
                 if ((arg & _BITMASK(16) ) != arg)
                 {
                     status = _ERROR;
                     break;
                 }
                 DMAC->stream3.max = arg;
                 break;
                 
            /* Set DMA memory to memory transfer source width 
            arg = 1,2,4 */   
            case DMA_SET_M2M_SRC_WIDTH:
                switch(arg)
                {
                    case 1:
                        DMAC->stream3.ctrl |= DMAC_CTRL_SOSIZE_1BYTE;
                        break;
                    case 2:
                        DMAC->stream3.ctrl |= DMAC_CTRL_SOSIZE_2BYTE;
                        break;
                    case 4:
                        DMAC->stream3.ctrl |= DMAC_CTRL_SOSIZE_4BYTE;
                        break;
                    default:
                        status = _ERROR;
                }
                 break;
                 
            /* Set DMA memory to memory transfer destination width
            arg = 1, 2, 4 */
            case DMA_SET_M2M_DEST_WIDTH:
                switch(arg)
                {
                    case 1:
                        DMAC->stream3.ctrl |= DMAC_CTRL_DESIZE_1BYTE;
                        break;
                    case 2:
                        DMAC->stream3.ctrl |= DMAC_CTRL_DESIZE_2BYTE;
                        break;
                    case 4:
                        DMAC->stream3.ctrl |= DMAC_CTRL_DESIZE_4BYTE;
                        break;
                    default:
                        status = _ERROR;
                }
                 break;
                 
            /* Set DMA memory to memory transfer burst length - Number of 
            items of src_width the DMAC can read from the source memory 
            in a single burst, must be 1, 4, 8, 16 */
            case DMA_SET_M2M_BURST_LEN:
                switch(arg)
                {
                    case 1:
                        DMAC->stream3.ctrl |= DMAC_CTRL_SOBURST_SINGLE;
                        break;
                    case 4:
                        DMAC->stream3.ctrl |= DMAC_CTRL_SOBURST_4INC;
                        break;
                    case 8:
                        DMAC->stream3.ctrl |= DMAC_CTRL_SOBURST_8INC;
                        break;
                    case 16:
                        DMAC->stream3.ctrl |= DMAC_CTRL_SOBURST_16INC;
                        break;
                    default:
                        status = _ERROR;
                }
                 break;
                 
            /* Start DMA memory to memory transfer */
            case DMA_START_M2M:
                /* chang the IOMUX to GPIO as this is mem2mem DMA mode */
                IOCON->mux_ctl_6 = IOCON_MUX6_PB1 | IOCON_MUX6_PB0;
    
                /* clear any previous mem2mem DMA flags */
                DMAC->clear = (DMAC_ERROR3 | DMAC_EOT3);

                DMAC->stream3.ctrl |= DMAC_CTRL_SOINC | DMAC_CTRL_DEINC |
                        DMAC_CTRL_ADDR_MODE_WRAP | DMAC_CTRL_MEM2MEM;

                DMAC->stream3.ctrl |= DMAC_CTRL_ENABLE;
            
                 break;
                 
            /* Stop DMA memory to memory transfer */
            case DMA_STOP_M2M:
                if ( (DMAC->stream3.ctrl & DMAC_CTRL_MEM2MEM) == 0)
                    status = _ERROR;
                else
                    DMAC->stream3.ctrl &= ~DMAC_CTRL_ENABLE;
                 break;
                 
            /* DMA set external request channel, it can be DREQ0 or DREQ1,
            arg = 0 set the DMA channel to be DREQ0, arg = 1 set the DMA
            channel to be DREQ1, this is the first function to be called
            before other external DMA setting function */
            case DMA_SET_EXT_CHANNEL:
                if (arg == 0)
                {
                    dma_ext_channel = 0;
                    /* chang the IOMUX to DMA pins for DREQ0, DACK0, DEOT0 */
                    IOCON->mux_ctl_6 = IOCON_MUX6_DREQ | IOCON_MUX6_DACK;
    
                }
                else if (arg == 1)
                {
                    dma_ext_channel = 1;
                    /* chang the IOMUX to DMA pins for DREQ0, DACK0, DEOT0 */
                    IOCON->mux_ctl_6 = IOCON_MUX6_DREQ | IOCON_MUX6_DACK;
                    
                }
                else
                {
                    if ((dma_ext_channel != 0) && (dma_ext_channel != 1))
                        dma_ext_channel = 0;
                    status = _ERROR;
                }
                 break;
                 
            /* DMA set external request direction, arg = 0 for peripheral
            to memory, arg = 1 for memory to peripheral */
            case DMA_SET_EXT_DIRECTION:
                if (arg == 0)
                {
                    /* peripheral to memory */
	                if (dma_ext_channel == 0)
    	            {
        	            /* wait for the DMA stream 2 to be idle */
            	        while (DMAC->status & DMAC_ACTIVE2);
                	    DMAC->stream2.ctrl &= ~DMAC_CTRL_PERIPH_DEST;
                	}
                	else
                	{
        	            /* wait for the DMA stream 3 to be idle */
            	        while (DMAC->status & DMAC_ACTIVE3);
                	    DMAC->stream3.ctrl &= ~DMAC_CTRL_PERIPH_DEST;
                	}
                    
                }
                else if (arg == 1)
                {
                    /* memory to peripheral */
	                if (dma_ext_channel == 0)
    	            {
	                    /* wait for the DMA stream 2 to be idle */
    	                while (DMAC->status & DMAC_ACTIVE2);
        	            DMAC->stream2.ctrl |= DMAC_CTRL_PERIPH_DEST;
        	        }
        	        else
        	        {
	                    /* wait for the DMA stream 3 to be idle */
    	                while (DMAC->status & DMAC_ACTIVE3);
        	            DMAC->stream3.ctrl |= DMAC_CTRL_PERIPH_DEST;
        	        }
                }
                else
                {
                    status = _ERROR;
                }
                 break;
                 
            /* DMA external set source address, arg = address */
            case DMA_SET_EXT_SRC:
                if (dma_ext_channel == 0)
                {
                    DMAC->stream2.sourcelo = arg & _BITMASK(16);
                    DMAC->stream2.sourcehi = (arg >> 16) & _BITMASK(16);
                }
                else if (dma_ext_channel == 1)
                {
                    DMAC->stream3.sourcelo = arg & _BITMASK(16);
                    DMAC->stream3.sourcehi = (arg >> 16) & _BITMASK(16);
                }
                else
                {
                    status = _ERROR;
                }
                 break;
                 
            /* DMA external set destination address, arg = address */
            case DMA_SET_EXT_DEST:
                if (dma_ext_channel == 0)
                {
                    DMAC->stream2.destlo = arg & _BITMASK(16);
                    DMAC->stream2.desthi = (arg >> 16) & _BITMASK(16);
                }
                else if (dma_ext_channel == 1)
                {
                    DMAC->stream3.destlo = arg & _BITMASK(16);
                    DMAC->stream3.desthi = (arg >> 16) & _BITMASK(16);
                }
                else
                {
                    status = _ERROR;
                }
                 break;
                 
            /* DMA external set number of bytes to be transfered, arg 
            = nbytes */
            /*   The number of bytes to transfer; nbytes / src_width
            must be less than 65536 and greater than 0 or this
            function will return failure. DMA_SET_EXT_SRC_WIDTH must
            be executed before this function call */
            case DMA_SET_EXT_BYTES:
                if (dma_ext_channel == 0)
                {
                    temp = DMAC->stream2.ctrl;
                    if (temp == DMAC_CTRL_SOSIZE_1BYTE)
                        temp = 1;
                    else if (temp == DMAC_CTRL_SOSIZE_2BYTE)
                        temp = 2;
                    else
                        temp = 4;   
                    arg /= temp;
                    if ((arg & _BITMASK(16) ) != arg)
                    {
                        status = _ERROR;
                        break;
                    }
                    DMAC->stream2.max = arg;
                }
                else if (dma_ext_channel == 1)
                {
                    temp = DMAC->stream3.ctrl;
                    if (temp == DMAC_CTRL_SOSIZE_1BYTE)
                        temp = 1;
                    else if (temp == DMAC_CTRL_SOSIZE_2BYTE)
                        temp = 2;
                    else
                        temp = 4;   
                    arg /= temp;
                    if ((arg & _BITMASK(16) ) != arg)
                    {
                        status = _ERROR;
                        break;
                    }
                    DMAC->stream3.max = arg;
                }
                else
                {
                    status = _ERROR;
                }
                
                 break;
                 
            /* DMA external transfer srouce width, arg = width */
            case DMA_SET_EXT_SRC_WIDTH:
                if (dma_ext_channel == 0)
                {
                    switch(arg)
                    {
                        case 1:
                            DMAC->stream2.ctrl |= DMAC_CTRL_SOSIZE_1BYTE;
                            break;
                        case 2:
                            DMAC->stream2.ctrl |= DMAC_CTRL_SOSIZE_2BYTE;
                            break;
                        case 4:
                            DMAC->stream2.ctrl |= DMAC_CTRL_SOSIZE_4BYTE;
                            break;
                        default:
                            status = _ERROR;
                    }
                }
                else if (dma_ext_channel == 1)
                {
                    switch(arg)
                    {
                        case 1:
                            DMAC->stream3.ctrl |= DMAC_CTRL_SOSIZE_1BYTE;
                            break;
                        case 2:
                            DMAC->stream3.ctrl |= DMAC_CTRL_SOSIZE_2BYTE;
                            break;
                        case 4:
                            DMAC->stream3.ctrl |= DMAC_CTRL_SOSIZE_4BYTE;
                            break;
                        default:
                            status = _ERROR;
                    }
                }
                else
                {
                    status = _ERROR;
                }
                 break;
                 
            /* DMA external transfer set destination width, arg = width */
            case DMA_SET_EXT_DEST_WIDTH:
                if (dma_ext_channel == 0)
                {
                    switch(arg)
                    {
                        case 1:
                            DMAC->stream2.ctrl |= DMAC_CTRL_DESIZE_1BYTE;
                            break;
                        case 2:
                            DMAC->stream2.ctrl |= DMAC_CTRL_DESIZE_2BYTE;
                            break;
                        case 4:
                            DMAC->stream2.ctrl |= DMAC_CTRL_DESIZE_4BYTE;
                            break;
                        default:
                            status = _ERROR;
                    }
                }
                else if (dma_ext_channel == 1)
                {
                    switch(arg)
                    {
                        case 1:
                            DMAC->stream3.ctrl |= DMAC_CTRL_DESIZE_1BYTE;
                            break;
                        case 2:
                            DMAC->stream3.ctrl |= DMAC_CTRL_DESIZE_2BYTE;
                            break;
                        case 4:
                            DMAC->stream3.ctrl |= DMAC_CTRL_DESIZE_4BYTE;
                            break;
                        default:
                            status = _ERROR;
                    }
                }
                else
                {
                    status = _ERROR;
                }
                 break;
            
            /* Set DMA transfer burst length - must be 1, 4, 8, 16
            1 for single, 4 for 4 increamenting, 8 for 8 increamenting,
            16 for 16 increamenting */
            case DMA_SET_EXT_BURST_LEN:
                if (dma_ext_channel == 0)
                {
                    switch(arg)
                    {
                        case 1:
                            DMAC->stream2.ctrl |= DMAC_CTRL_SOBURST_SINGLE;
                            break;
                        case 4:
                            DMAC->stream2.ctrl |= DMAC_CTRL_SOBURST_4INC;
                            break;
                        case 8:
                            DMAC->stream2.ctrl |= DMAC_CTRL_SOBURST_8INC;
                            break;
                        case 16:
                            DMAC->stream2.ctrl |= DMAC_CTRL_SOBURST_16INC;
                            break;
                        default:
                            status = _ERROR;
                    }
                }
                else if (dma_ext_channel == 1)
                {
                    switch(arg)
                    {
                        case 1:
                            DMAC->stream3.ctrl |= DMAC_CTRL_SOBURST_SINGLE;
                            break;
                        case 4:
                            DMAC->stream3.ctrl |= DMAC_CTRL_SOBURST_4INC;
                            break;
                        case 8:
                            DMAC->stream3.ctrl |= DMAC_CTRL_SOBURST_8INC;
                            break;
                        case 16:
                            DMAC->stream3.ctrl |= DMAC_CTRL_SOBURST_16INC;
                            break;
                        default:
                            status = _ERROR;
                    }
                }
                else
                {
                    status = _ERROR;
                }
                 break;
                 
            /* DMA external set current source register increamented.
            arg = 1, increamented, arg = 0, unchanged */
            case DMA_SET_SO_INC:
                if (dma_ext_channel == 0)
                {
	                if(arg == 1)
	                {
		                /* current source register increamented */
	                    DMAC->stream2.ctrl |= DMAC_CTRL_SOINC;
	                }
	                else
	                {
		                /* current source register unchanged */
	                    DMAC->stream2.ctrl &= ~DMAC_CTRL_SOINC;
	                }
                }
                else if (dma_ext_channel == 1)
                {
	                if(arg == 1)
	                {
		                /* current source register increamented */
	                    DMAC->stream3.ctrl |= DMAC_CTRL_SOINC;
	                }
	                else
	                {
		                /* current source register unchanged */
	                    DMAC->stream3.ctrl &= ~DMAC_CTRL_SOINC;
	                }
                }
                else
                {
                    status = _ERROR;
                }
                 break;
                 
            /* DMA external set current destination register increamented.
            arg = 1, increamented, arg = 0, unchanged */
            case DMA_SET_DE_INC:
                if (dma_ext_channel == 0)
                {
	                if(arg == 1)
	                {
		                /* current source register increamented */
	                    DMAC->stream2.ctrl |= DMAC_CTRL_DEINC;
	                }
	                else
	                {
		                /* current source register unchanged */
	                    DMAC->stream2.ctrl &= ~DMAC_CTRL_DEINC;
	                }
                }
                else if (dma_ext_channel == 1)
                {
	                if(arg == 1)
	                {
		                /* current source register increamented */
	                    DMAC->stream3.ctrl |= DMAC_CTRL_DEINC;
	                }
	                else
	                {
		                /* current source register unchanged */
	                    DMAC->stream3.ctrl &= ~DMAC_CTRL_DEINC;
	                }
                }
                else
                {
                    status = _ERROR;
                }
                 break;
                 
            /* DMA start external transfer, arg = 1, enable. 
            arg = 0, disable */
            case DMA_ENABLE_EXT:
                if (dma_ext_channel == 0)
                {
	                /* Enable the external DMA - channel 0 */
	                if(arg == 1)
	                {
                    	/* clear any previous DMA flags */
                    	DMAC->clear = (DMAC_ERROR2 | DMAC_EOT2);
                    
                    	DMAC->stream2.ctrl |= DMAC_CTRL_ADDR_MODE_WRAP;

                    	DMAC->stream2.ctrl |= DMAC_CTRL_ENABLE;
                    }
                    /* Disable the external DMA - channel 0*/
                    else
                    {
                    	DMAC->clear = (DMAC_ERROR2 | DMAC_EOT2);
                    
                        DMAC->stream2.ctrl &= ~DMAC_CTRL_ENABLE;
                    }
                }
                else if (dma_ext_channel == 1)
                {
	                /* Enable the external DMA - channel 1 */
	                if(arg == 1)
	                {
	                    /* clear any previous DMA flags */
    	                DMAC->clear = (DMAC_ERROR3 | DMAC_EOT3);
                    
        	            DMAC->stream3.ctrl |= DMAC_CTRL_ADDR_MODE_WRAP;
	
    	                DMAC->stream3.ctrl |= DMAC_CTRL_ENABLE;
    	            }
                    /* Disable the external DMA - channel 1 */
    	            else
    	            {
                    	DMAC->clear = (DMAC_ERROR3 | DMAC_EOT3);
                    
                        DMAC->stream3.ctrl &= ~DMAC_CTRL_ENABLE;
    	            }
                }
                else
                {
                    status = _ERROR;
                }
                 break;
                 
            /* Get DMA status, status = 
                DMAC_EOT0
                DMAC_EOT1
                DMAC_EOT2
                DMAC_EOT3
                DMAC_ERROR0
                DMAC_ERROR1
                DMAC_ERROR2
                DMAC_ERROR3
                DMAC_ACTIVE0 
                DMAC_ACTIVE1 
                DMAC_ACTIVE2 
                DMAC_ACTIVE3 
            */          
            case DMA_GET_STATUS:
                status = (DMAC->status & 0x0fff);   

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
 * Function: dma_read
 *
 * Purpose: DMA read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to DMA config structure
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
INT_32 dma_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
    return 0;
}

/***********************************************************************
 *
 * Function: dma_write
 *
 * Purpose: DMA write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid: Pointer to DMA config structure
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
INT_32 dma_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
    return 0;
}
