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
* @file name	csp_ldma.c
* @description	Function declarations for LDMA module
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
*		LDMA/ /S3F4A0K/AGP1 EVB Rev1.0
*/ 


/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"


/*******************************************************************************
****************************  F U N C T I O N S  *******************************
*******************************************************************************/

/**
********************************************************************************
* @description	Reset registers and state machines.
*
* @return	None		
*
* @param    ldmac	[in] Pointer to LDMAC structure.
* 
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_LDMAInit(CSP_LDMA_T *const ldma)
{
   /* Software Reset */
   CSP_LDMA_SET_CR(ldma,LDMA_SWRST);

}

/**
********************************************************************************
* @description	Init the mode for a specific channel for Rx.
*
* @return	None
*
* @param    ldmac 				[in] Pointer to LDMAC structure.
*			periphAddress_u32	[in] Pointer to LDMAC structure.
*			channel_u8			[in] channel whitch will be configured.
*			mode_u32			[in] Configure the channel mode.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_LDMARxInitChannel(CSP_LDMA_T *const ldma, 
                            U32_T periphAddress_u32, 
                            U8_T channel_u8, 
                            U32_T mode_u32)
{
   /* Disable Channel */
   CSP_LDMA_SET_CRX(ldma, channel_u8, LDMA_LCHDIS);
   
   /* Set LDMA mode register for a specific channel */
   CSP_LDMA_SET_MRX(ldma, channel_u8, (mode_u32 |
                     LDMA_CHREADY(CSP_GET_LDMA_RX_NUMBER(periphAddress_u32))
                     ));
}

/**
********************************************************************************
* @description	Init the mode for a specific channel for Tx.
*
* @return	None
*
* @param    ldmac 				[in] Pointer to LDMAC structure.
*			periphAddress_u32	[in] Pointer to LDMAC structure.
*			channel_u8			[in] channel whitch will be configured.
*			mode_u32			[in] Configure the channel mode.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_LDMATxInitChannel(CSP_LDMA_T *const ldma, 
                            U32_T periphAddress_u32, 
                            U8_T channel_u8, 
                            U32_T mode_u32)
{
   /* Disable Channel */
   CSP_LDMA_SET_CRX(ldma, channel_u8, LDMA_LCHDIS);
   
   /* Set LDMA mode register for a specific channel */
   CSP_LDMA_SET_MRX(ldma, channel_u8, (mode_u32 |
                     LDMA_CHREADY(CSP_GET_LDMA_TX_NUMBER(periphAddress_u32))
                     ));
}

/**
********************************************************************************
* @description	Configure LDMA interrupt.
*
* @return	None
*
* @param    ldmac 				[in] Pointer to LDMAC structure.
*			intMask_u32			[in] Configure the interrupt mask.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_LDMAConfigInterrupt(CSP_LDMA_T *const ldma, 
                              U32_T intMode_u32, 
                              U32_T intMask_u32, 
                              U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum_u8 = 0;

   /* Get Peripheral Interrupt Number */
   intNum_u8 = CSP_GET_SYS_INT_NUMBER(ldma);

   /* Disable all interrupt */
   CSP_LDMA_SET_IDR(ldma, 0xFFFFFFFF);
      
   /* Interrupt Enable */
   CSP_LDMA_SET_IER(ldma, intMask_u32);
   
   /* Configure EPC controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);

}


/**
********************************************************************************
* @description	Configure channel interrupt.
*
* @return	None
*
* @param    ldmac 				[in] Pointer to LDMAC structure.
*			channel_u8			[in] channel whitch will be configured.
*			intChannelMask_u32  [in] Configure which interrupt bits 
*									 are activated for the channel.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_LDMAConfigChannelInterrupt(CSP_LDMA_T *const ldma,
                                    U8_T channel_u8,
                                    U32_T intChannelMask_u32)
{
   /* Enable interrupt for the specified channel */
   CSP_LDMA_SET_IERX(ldma, channel_u8, intChannelMask_u32);

   /* Clear interrupt */
   CSP_LDMA_SET_CSRX(ldma, channel_u8, intChannelMask_u32);
}

/**
********************************************************************************
* @description	Enable a LDMA channel.
*
* @return	None
*
* @param    ldmac 				[in] Pointer to LDMAC structure.
*			channel_u8			[in] channel whitch will be configured.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_LDMAEnableChannel(CSP_LDMA_T *const ldma, U8_T channel_u8)
{
   /* Enable Channel */
   CSP_LDMA_SET_CRX(ldma, channel_u8, LDMA_LCHEN);
}

/**
********************************************************************************
* @description	Disable a LDMA channel.
*
* @return	None
*
* @param    ldmac 				[in] Pointer to LDMAC structure.
*			channel_u8			[in] channel whitch will be configured.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_LDMADisableChannel(CSP_LDMA_T *const ldma, U8_T channel_u8)
{
   /* Disable Channel */
   CSP_LDMA_SET_CRX(ldma, channel_u8, LDMA_LCHDIS);
}

/**
********************************************************************************
* @description	Configure and start a transfer.
*
* @return	None
*
* @param    ldmac 				[in] Pointer to LDMAC structure.
*			channel_u8			[in] channel whitch will be configured.
*			srcAddress_u32		[in] source address.
*			destAddress_u32		[in] destination address.
*			nbData_u32			[in] number of data to be tranfered.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_LDMAStartTransfer(CSP_LDMA_T *const ldma, U8_T channel_u8,
                            U32_T srcAddress_u32, U32_T destAddress_u32,
                            U16_T nbData_u16)
{
   /* Disable channel */
   CSP_LDMA_SET_CRX(ldma, channel_u8, LDMA_LCHDIS);

   /* Set the source address */
   CSP_LDMA_SET_ASRCRX(ldma, channel_u8, srcAddress_u32);

   /* Set the destination address */
   CSP_LDMA_SET_ADSTRX(ldma, channel_u8, destAddress_u32);

   /* Set the number of data to be transferred */
   CSP_LDMA_SET_CNTRX(ldma, channel_u8, nbData_u16);

   /* Enable Channel */
   CSP_LDMA_SET_CRX(ldma, channel_u8, LDMA_LCHEN);
}
