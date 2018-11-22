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
* @file name	csp_usart.c
* @description	Function declarations for USART module
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
*		USART/ /S3F4A0K/AGP1 EVB Rev1.0
*/



/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"

/*
*******************************************************************************
****************************  F U N C T I O N S  ******************************
*******************************************************************************
*/


/**
********************************************************************************
* @description	Switch on the clock, reset the registers and configure 
*               the USART module mode and the LDMA RX/TX line
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*			mode_u32		[in] Configure the USART mode.
*			channel_u8		[in] Configure USART LDMAC Channel
*			baudrate		[in] Configure the Baud Rate Generator.
*			time_out		[in] Configure the receiver time-out :
*                                the time-out is loaded when the Start 
*                                Time-out Command is given or when                   
*                                each new character is received.
*			time_guard		[in] Configure the transmit time-guard : TXD is
*                                inactive (high level) after the transmission
*                                of each character for the time-guard duration
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTInit(CSP_USART_T *const usart, 
                   U32_T mode_u32,
                   U8_T txChannel_u8,                   
                   U8_T rxChannel_u8, 
                   U16_T baudrate_u16, 
                   U8_T timeOut_u8, 
                   U8_T timeGuard_u8)
{
   /* Local Variables */
   U8_T transferSize_u8 = 0;

   /* Enable USART Clock */
   CSP_USART_SET_ECR(usart, USART_USART);
   
   /* USART Software Reset */
   CSP_USART_SET_CR(usart, (USART_SWRST | USART_RSTRX | USART_RSTTX));   

   /* Configure USART Mode */
   CSP_USART_SET_MR(usart, mode_u32);
    
   /* Set Receiver Time Out */
   CSP_USART_SET_RTOR(usart, timeOut_u8);
  
   /* Set Transmitter Timer Guard */
   CSP_USART_SET_TTGR(usart, timeGuard_u8);
  
   /* Set Baudrate */
   CSP_USART_SET_BRGR(usart, baudrate_u16);

   /* Configure the transfer size in accordance with the Mode Register */
   if((mode_u32 & USART_MODE9) == 0)
   {
      transferSize_u8 = LDMA_BYTE_SIZE;
   }
   else
   {
      transferSize_u8 = LDMA_HALF_WORD_SIZE;
   }

   /* LDMAC RX Definition */
   CSP_LDMARxInitChannel(LDMA0, (U32_T)usart, rxChannel_u8, 
                          (LDMA_DEST | LDMA_DEST_INCR | LDMA_TRIG | 
                           LDMA_SIZE(transferSize_u8))); 
   
   /* LDMAC TX Definition */ 
   CSP_LDMATxInitChannel(LDMA0, (U32_T)usart, txChannel_u8, 
                          (LDMA_SRC | LDMA_SRC_INCR | LDMA_TRIG | 
                           LDMA_SIZE(transferSize_u8)));

}


/**
********************************************************************************
* @description	Switch on the clock, reset the registers and configure 
*               the USART module for LIN mode
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*			baudrate_u16	[in] Configure the Baud Rate Generator.
*			LinVersion_u32	[in] Configure the LIN version 1.2 or 2.0
*			TimeGuard_u8	[in] Configure the transmit time-guard 
*                                (interbyte space)
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTInitLin(CSP_USART_T *const usart, 
                      U16_T baudrate_u16, 
                      U32_T linVersion_u32, 
                      U8_T timeGuard_u8)
{

   /* Enable USART & PIO Clock */
   CSP_USART_SET_ECR(usart, USART_USART);

   /* USART Software Reset */
   CSP_USART_SET_CR(usart, (USART_SWRST | USART_RSTRX | USART_RSTTX));
   

   /* Configure LIN Mode */
   CSP_USART_SET_MR(usart,  (linVersion_u32 | USART_CHMODE_NORMAL |
                             USART_NBSTOP_1 |USART_PAR_NO | USART_ASYNC |
                             USART_CHRL_8 | USART_CLKS_MCKI | USART_LIN));
   
   /* Set Baudrate */
   CSP_USART_SET_BRGR(usart, baudrate_u16);
    
   /* Set Transmitter Timer Guard */
   CSP_USART_SET_TTGR(usart, timeGuard_u8);
}


/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTClose(CSP_USART_T *const usart)
{
   /* USART Software Reset */
   CSP_USART_SET_CR(usart, (USART_SWRST | USART_RSTRX | USART_RSTTX));

   /* Disable USART Clock */
   CSP_USART_SET_DCR(usart, USART_USART);
}


/**
********************************************************************************
* @description	Configure USART Interrupts
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*			intMode_u32		[in] Configure the priority level and source type.
*			intMask_u32		[in] Configure which interrupt bits are activated.
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
void CSP_USARTConfigInterrupt(CSP_USART_T *const usart, 
                              U32_T intMode_u32, 
                              U32_T intMask_u32, 
                              U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum_u8 = 0;

   /* Get Peripheral Interrupt Number */
   intNum_u8 = CSP_GET_INT_NUMBER(usart);

   /* Disable all interrupt */
   CSP_USART_SET_IDR(usart, 0xFFFFFFFF);
   
   /* Interrupt Enable */
   CSP_USART_SET_IER(usart, intMask_u32);

   /* Configure USART controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}


/**
********************************************************************************
* @description	Enable USART
*
* @return	None		
*
* @param    usart				[in] Pointer to USART structure.
*			enableMask_u32		[in] Configure which functions (RX and/or TX) 
*                                    are enabled
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTEnable(CSP_USART_T *const usart, U32_T enableMask_u32)
{
   /* USART Rx and/or Tx Enable */
   CSP_USART_SET_CR(usart, enableMask_u32);
}


/**
********************************************************************************
* @description	Disable USART
*
* @return	None		
*
* @param    usart				[in] Pointer to USART structure.
*			disableMask_u32		[in] Configure which functions (RX and/or TX) 
*                                    are disabled
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTDisable(CSP_USART_T *const usart, U32_T disableMask_u32)
{
   /* USART Rx and/or Tx Disable */
   CSP_USART_SET_CR(usart, disableMask_u32);
}
 

/**
********************************************************************************
* @description	Configure USART to receive data. They will be transferred 
*               automatically to memory thanks to LDMA.
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*			channel_u8   	[in] Configure USART LDMAC Channel
*			data_u8 	   	[in] Pointer to an array where data packet 
*                                received will be store
*			length_u16   	[in] Number of byte to receive
*
* @pre		None
* @post	    You should wait the set of the ENDRX bit in SR.
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTReceive(CSP_USART_T *const usart, 
                      U8_T channel_u8, 
                      U32_T destAddress_u32, 
                      U16_T length_u16)
{
   /* Disable Reception */
   CSP_USART_SET_CR(usart, USART_RXDIS);

   /* Configure LDMAC Rx Line to Receive a packet */
   CSP_LDMAStartTransfer(LDMA0, channel_u8, ((U32_T)usart | 0x80), 
                           destAddress_u32, length_u16);    
  
   /* Clear Status */
   CSP_USART_SET_CR(usart, USART_STTTO);

   /* Enable Reception (if RTOR = 0 time out is disabled) */
   CSP_USART_SET_CR(usart, USART_RXEN);
}


/**
********************************************************************************
* @description	Configure USART to transmit data. They are transferred 
*               automatically from memory to USART transmission buffer thanks 
*               to LDMA.
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*			channel_u8 		[in] Configure USART LDMAC Channel
*			data_u8 		[in] Pointer to an array where data packet 
*                                received will be store
*			length_u16 		[in] Number of byte to receive
*
* @pre		None
* @post	    You should wait the set of the ENDTX bit in SR.
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTTransmit(CSP_USART_T *const usart, 
                       U8_T channel_u8, 
                       U32_T sourceAddress_u32, 
                       U16_T length_u16)
{
   /* Disable Transmission */
   CSP_USART_SET_CR(usart, USART_TXDIS);

   /* Configure LDMAC Tx Line to Send a packet */
   CSP_LDMAStartTransfer(LDMA0, channel_u8, sourceAddress_u32,
                           ((U32_T)usart | 0x84), length_u16); 
     
   /* Enable Transmission */
   CSP_USART_SET_CR(usart, USART_TXEN);
}



/**
********************************************************************************
* @description	Transmit a LIN header Frame
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*			sblr_u8			[in] Configure the Synchro Break Length 
*			LinMode_u32		[in] Configure the LIN's mode
*
* @pre		None
* @post	    You should wait the set of the ENDHEADER bit in SR.
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTTransmitLinHeaderFrame(CSP_USART_T *const usart, U8_T sblr_u8, 
                                     U32_T linMode_u32)
{
   /* Configure the Sync Break Length */
   CSP_USART_SET_SBLR(usart, sblr_u8);
   
   /* Configure LIN mode */
   CSP_USART_SET_LIR(usart, linMode_u32);

   /* Transmit Header */
   CSP_USART_SET_CR(usart, USART_STHEADER);
}


/**
********************************************************************************
* @description	Initialise Data Field Write Registers for LIN Response Frame 
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*			data_u8			[in] Pointer to an array where data packet 
*             					 sent is stored
*
* @pre		You should use this function before the 
*           CSP_USARTTransmitLinHeaderFrame function in case of 
*           a Master Request Frame.
*
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTLinResponseFrameInit(CSP_USART_T *const usart, U8_T *data_u8)
{
   
   /* The writing in DFWR must be in 32 bits */
   CSP_USART_SET_DFWR0(usart, (*(U32_T*)&data_u8[0]));
   CSP_USART_SET_DFWR1(usart, (*(U32_T*)&data_u8[4]));

}


/**
********************************************************************************
* @description	Transmit a LIN Response Frame 
*
* @return	None		
*
* @param    usart			[in] Pointer to USART structure.
*
* @pre		You should use the CSP_USARTTransmitLinHeaderFrame function
*           before using this function.
*
* @post	    You should wait the set of the ENDMESS bit in SR.
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_USARTTransmitLinRpseFrame(CSP_USART_T *const usart)
{
   /* Transmit Response */
   CSP_USART_SET_CR(usart, USART_STRESP);

}







