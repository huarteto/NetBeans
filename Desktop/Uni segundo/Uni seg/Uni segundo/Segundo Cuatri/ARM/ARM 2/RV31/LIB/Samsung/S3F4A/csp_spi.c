/*
* Automotive Body/DashBoard Application MCU,S3F4A0K,
* Developed by Microcontroller Design Team
* 
* Copyright 2005 by MCU System Team, Samsung Electronics CO.,LTD.
* San #24 Nongseo-Dong,Giheung-Gu,
* 449-711 Youngin-City, Gyeongi-Do, Korea.
* All rights reserved.
*
* This software is the confidential and proprietary information of Samsung
* Electronics CO.,LTD. ("Confidential Information"). 
* You shall not disclose such Confidential Inforamtion and shall use it only
* in accordance with the terms of the license agreement you entered into
* with Samsung.
* 
* The software is delivered "AS IS" without warranty or condition of any 
* kind, either express, implied or statutory. This includes without limitation
* any warranty or condition with respect to merchantability or fitness for
* any particular purpose, or against the infringements of intellectual property
* rights of others. 
*/

/**
* @file name	csp_spi.c
* @description	Function declarations for Serial Peripheral Interface module
*                
* @author	Juil Kim(82-31-209-2379, prodakim@samsung.com)   
*			Younghee Jin(82-31-209-4956, como.jin@samsung.com)
*			Minseok Jeong(82-31-209-3865, ms3525.jeong@samsung.com)		
*
* @version	Preliminary 0.0  
* @history	History type - NEW/MODify/ADD/DELete  
*  			 -------------------------------------------------------------------
*			|ver type when       who                       what             
*			|---+---+----------+-------------------------+----------------------
*			|0.0 NEW 01/07/2005  Samsung MCU System Team   Creation			
*	 		 -------------------------------------------------------------------
*
* @see	ADS Compiler Tool
*		Module/Drivers/Processor/Board -
*		SPI/ /S3F4A0K/AGP1 EVB Rev1.0
*/

/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"



/******************************************************************************
****************************  F U N C T I O N S  ******************************
******************************************************************************/

/**
********************************************************************************
* @description	Switch on the clock, reset the registers and configure 
*               the SPI module mode
*
* @return	None		
*
* @param    spi				[in] Pointer to SPI structure.
*			mode_u32		[in] Configure the SPI mode.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SPIInit(CSP_SPI_T *const spi, U32_T mode_u32)
{
   /* Enable SPI Clock */
   CSP_SPI_SET_ECR(spi, SPI_SPI);
   
   /* SPI Software Reset */
   CSP_SPI_SET_CR(spi, SPI_SWRST);

   /* Configure SPI Mode */
   CSP_SPI_SET_MR(spi, mode_u32);
}


/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None		
*
* @param    spi				[in] Pointer to SPI structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SPIClose(CSP_SPI_T *const spi)
{
   /* SPI Software Reset */
   CSP_SPI_SET_CR(spi, SPI_SWRST);

   /* Disable SPI Clock */
   CSP_SPI_SET_DCR(spi, SPI_SPI );
}


/**
********************************************************************************
* @description	Configure the SPI Chip Select 
*
* @return	None		
*
* @param    spi				[in] Pointer to SPI structure.
*			cs_u8			[in] Chip Select [0-3] to be configured.
*			mode_u32		[in] Configure the SPI Chip Select mode.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SPIConfigureCS(CSP_SPI_T *const spi, U8_T cs_u8, U32_T mode_u32)
{   
   /* Configure SPI CS mode */
   CSP_SPI_CS_SET_SSR(spi, cs_u8, mode_u32);
}


/**
********************************************************************************
* @description	Configure the SPI 
*
* @return	None		
*
* @param    spi				[in] Pointer to SPI structure.
*			intMode_u32		[in] Configure the priority level and source type.
*			intMask_u32		[in] Configure which interrupt bits are activated
*			callback_u32	[in] Function called through the assembler 
*                                interrupt handler.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SPIConfigInterrupt(CSP_SPI_T *const spi, 
                            U32_T intMode_u32, 
                            U32_T intMask_u32, 
                            U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum = 0;

   /* Get Peripheral Interrupt Number */
   intNum = CSP_GET_INT_NUMBER(spi);

   /* Disable all interrupt */
   CSP_SPI_SET_IDR(spi, 0xFFFFFFFF);
   
   /* Interrupt Enable */
   CSP_SPI_SET_IER(spi, intMask_u32);

   /* Configure SPI controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum, intMode_u32, callback_u32);
}


/**
********************************************************************************
* @description	Enable SPI
*
* @return	None		
*
* @param    spi				[in] Pointer to SPI structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SPIEnable(CSP_SPI_T *const spi)
{
   /* Enable SPI */
   CSP_SPI_SET_CR(spi, SPI_SPIEN);
}


/**
********************************************************************************
* @description	Disable SPI
*
* @return	None		
*
* @param    spi				[in] Pointer to SPI structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SPIDisable(CSP_SPI_T *const spi)
{
   /* Disable SPI */
   CSP_SPI_SET_CR(spi, SPI_SPIDIS);
}


/**
********************************************************************************
* @description	Configure SPI to receive data. They are transferred
*               automatically thanks to LDMA
*
* @return	None		
*
* @param    spi				[in] Pointer to SPI structure.
*			cs_u8			[in] Receiver Chip Select [0-3]
*							NOTE: The chip select is only available 
*								  with SPI16 peripheral
*			channel_u8		[in] Configure SPI LDMA Channel
*			destAddress_u32	[in] Pointer to an array where command packet sent 
*                                is stored.
*			length_u16		[in] Number of data packets to exchange.
*							NOTE: The data packet length depends on the BITS 
*								  mode specified by the CSP_SPIConfigureCS 
*								  function.
*
* @pre		CSP_SPIInit, CSP_SPIConfigureCS
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SPIReceive(CSP_SPI_T *const spi, 
                    U8_T cs_u8, 
                    U8_T channel_u8, 
                    U32_T destAddress_u32, 
                    U16_T length_u16)
{  
   /* Local Variables */
   U8_T transferSize_u8 = 0;
   U32_T readBackCs0Bits_u32;
   U32_T cs0Mode_u32;

    /* Get the type of peripheral select */
    if((CSP_SPI_GET_MR(spi) & SPI_PS) == SPI_PS)
    {    /* THIS IS A SPI16 MODULE */
    
        /* Variable peripheral select : 16 bits Data + PCS = 32 bits */
        /* This feature is only available with SPI16                 */
        /*-----------------------------------------------------------*/
        transferSize_u8 = LDMA_HALF_WORD_SIZE;
    }
    else
    {
        /* Identify if it is SPI8 or SPI16 ? */
        /* This part of code shall be removed when an identifier will be */
        /* available for each module */
        
        /* Save SSR0 register which was previously configured using */
        /* CSP_SPIConfigureCS */
        cs0Mode_u32 = CSP_SPI_CS_GET_SSR(spi,CS0);
        
        /* Set all bits of CS0 to 1 in BITS field to check if the MSB is */
        /* set to 1 => SPI16 else => SPI8 */
        CSP_SPI_CS_SET_SSR(spi, CS0, SPI_BITS_MASK);
        
        /* Read back all bits of CS0  in BITS field and check if the MSB is */
        /* set to 1 => SPI16 else => SPI8 */
        readBackCs0Bits_u32 = CSP_SPI_CS_GET_SSR(spi, CS0);

        /* Restore SSR0 register which was previously configured */
        /* using CSP_SPIConfigureCS */
        CSP_SPI_CS_SET_SSR(spi, CS0, cs0Mode_u32);
        
        
        if ((readBackCs0Bits_u32 & SPI_BITS_MASK) == SPI_BITS_MASK)
        {    /* THIS IS A SPI16 MODULE */

            /* With SPI16 the DATA transfer size depends on nb of */ 
            /* bits per transfer */         
            /* Get the transfer size from BITS field of selected CSx */
            if ((CSP_SPI_CS_GET_SSR(spi, cs_u8) & SPI_BITS_MASK) == SPI_BITS_8)
            {
                /* 8 bits Data transfer with SPI16 */
                transferSize_u8 = LDMA_BYTE_SIZE;
            }
            else
            {
                /* 16 bits Data transfer */
                transferSize_u8 = LDMA_HALF_WORD_SIZE;
            }
        } 
        else
        {    /* THIS IS A SPI8 MODULE */
                
            /* With SPI8 only 8 bits DATA transfer is possible */
            transferSize_u8 = LDMA_BYTE_SIZE;
        }
    }
           
   /* LDMA RX Definition */
   CSP_LDMARxInitChannel(LDMA0, (U32_T)spi, channel_u8, (LDMA_DEST |
                          LDMA_DEST_INCR | LDMA_TRIG | 
                          LDMA_SIZE(transferSize_u8))); 
      
   /* Enable SPI */
   CSP_SPI_SET_CR(spi, SPI_SPIEN);
  
   /* Start RX Dialog */
   CSP_LDMAStartTransfer(LDMA0, channel_u8, ((U32_T)spi | 0x80), 
                          destAddress_u32, length_u16);

}


/**
********************************************************************************
* @description	Configure SPI to transmit data. They are transferred 
*               automatically from memory to SPI transmission buffer 
*				thanks to LDMA
*
* @return	None		
*
* @param    spi					[in] Pointer to SPI structure.
*			cs_u8				[in] Receiver Chip Select [0-3]
*								NOTE: The chip select is only available 
*								  	  with SPI16 peripheral
*			channel_u8			[in] Configure SPI LDMA Channel
*			sourceAddress_u32	[in] Pointer to an array where command packet 
									 sent is stored.
*			length_u16			[in] Number of data packets to exchange.
*								NOTE: The data packet length depends on the BITS 
*								      mode specified by the CSP_SPIConfigureCS 
*								      function.
*
* @pre		CSP_SPIInit, CSP_SPIConfigureCS
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SPITransmit(CSP_SPI_T *const spi, 
                     U8_T cs_u8, 
                     U8_T channel_u8, 
                     U32_T sourceAddress_u32, 
                     U16_T length_u16)
{

  
    /* Local Variables */
    U8_T transferSize_u8 = 0;
    U32_T readBackCs0Bits_u32;
    U32_T cs0Mode_u32;

    /* Get the type of peripheral select */
    if((CSP_SPI_GET_MR(spi) & SPI_PS) == SPI_PS)
    {    /* THIS IS A SPI16 MODULE */
    
        /* Variable peripheral select : 16 bits Data + PCS = 32 bits */
        /* This feature is only available with SPI16                 */
        /*-----------------------------------------------------------*/
        transferSize_u8 = LDMA_HALF_WORD_SIZE;
    }
    else
    {       
        /* Identify if it is SPI8 or SPI16 ? */
        /* This part of code shall be removed when an identifier will be */ 
        /* available for each module */
        
        /* Save the BITS field of SSR0 register which was previously */
        /* configured using CSP_SPIConfigureCS */
        cs0Mode_u32 = CSP_SPI_CS_GET_SSR(spi,CS0);
        
        /* Set all bits of CS0 to 1 in BITS field to check if the MSB is */ 
        /* set to 1 => SPI16 else => SPI8 */
        CSP_SPI_CS_SET_SSR(spi, CS0, SPI_BITS_MASK);
        
        /* Read back all bits of CS0  in BITS field and check if the MSB is */
        /* set to 1 => SPI16 else => SPI8 */
        readBackCs0Bits_u32 = CSP_SPI_CS_GET_SSR(spi, CS0);

        /* Restore the BITS field of SSR0 register which was previously */ 
        /* configured using CSP_SPIConfigureCS */
        CSP_SPI_CS_SET_SSR(spi, CS0, cs0Mode_u32);
        
        
        if ((readBackCs0Bits_u32 & SPI_BITS_MASK) == SPI_BITS_MASK)
        {    /* THIS IS A SPI16 MODULE */

            /* With SPI16 the DATA transfer size depends on nb of */
            /* bits per transfer */         
            /* Get the transfer size from BITS field of selected CSx */
            if ((CSP_SPI_CS_GET_SSR(spi, cs_u8) & SPI_BITS_MASK) == SPI_BITS_8)
            {
                /* 8 bits Data transfer with SPI16 */
                transferSize_u8 = LDMA_BYTE_SIZE;
            }
            else
            {
                /* 16 bits Data transfer */
                transferSize_u8 = LDMA_HALF_WORD_SIZE;
            }
        } 
        else
        {    /* THIS IS A SPI8 MODULE */
                
            /* With SPI8 only 8 bits DATA transfer is possible */
            transferSize_u8 = LDMA_BYTE_SIZE;
        }
    }
       
   /* LDMA Definition */
   CSP_LDMATxInitChannel(LDMA0, (U32_T)spi, channel_u8, (LDMA_SRC |
                          LDMA_SRC_INCR | LDMA_TRIG | 
                          LDMA_SIZE(transferSize_u8))); 
      
   /* Enable SPI */
   CSP_SPI_SET_CR(spi, SPI_SPIEN);
   
   /* Start Dialog */
   CSP_LDMAStartTransfer(LDMA0, channel_u8, sourceAddress_u32, 
                           ((U32_T)spi | 0x84), length_u16);


}








