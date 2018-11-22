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
* @file name	csp_st16.c
* @description	Function declarations for Simple Timer module
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
*		ST/ /S3F4A0K/AGP1 EVB Rev1.0
*/

/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"


/******************************************************************************
****************************  F U N C T I O N S  ******************************
******************************************************************************/


/**
*******************************************************************************
* @brief     Switch on the clock and reset the registers 
*
* @retval    None
*
* @param     st
* \n          input
* \n          brief description - Pointer to ST structure.
*
* @pre       None
* @post      None
*
* @todo      Nothing
*
* @par InBodyDocs:
*******************************************************************************
*/
void CSP_ST16Init(CSP_ST16_T *const st)
{
   /* Enable ST Clock */
   CSP_ST16_SET_ECR(st, ST16_ST);

   /* ST Software Reset */
   CSP_ST16_SET_CR(st, ST16_SWRST);
}

/**
********************************************************************************
* @description	Reset and switch off the clock 
*
* @return	None		
*
* @param    st				[in] Pointer to ST structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ST16Close(CSP_ST16_T *const st)
{
   /* ST Software Reset */
   CSP_ST16_SET_CR(st, ST16_SWRST);

   /* Disable ST Clock */
   CSP_ST16_SET_DCR(st, ST16_ST);
}


/**
********************************************************************************
* @description	Configure the ST channel
*
* @return	None		
*
* @param    st				[in] Pointer to ST structure.
*			channel_u8		[in] Channel [0-7] to be configured.
*			prescalMode_u32	[in] Configure the channel prescalar mode.
*			loadCounter_u32	[in] Configure the counter value.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ST16ConfigureChannel(CSP_ST16_T *const st, 
                              U8_T channel_u8, 
                              U32_T prescalMode_u32, 
                              U32_T loadCounter_u32)
{
   /* Configure Counter Value */
   CSP_ST16_CHANNEL_SET_CT(st, channel_u8, loadCounter_u32);
        
   /* Configure Prescalar Value */
   CSP_ST16_CHANNEL_SET_PR(st, channel_u8, prescalMode_u32);
}


/**
********************************************************************************
* @description	Configure ST Interrupts
*
* @return	None		
*
* @param    st				[in] Pointer to ST structure.
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
void CSP_ST16ConfigInterrupt(CSP_ST16_T *const st, 
                             U32_T intMode_u32, 
                             U32_T intMask_u32, 
                             U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum_u8 = 0;

   /* Get Peripheral Interrupt Number */
   intNum_u8 = CSP_GET_INT_NUMBER(st);

   /* Disable all interrupt */
   CSP_ST16_SET_IDR(st, 0xFFFFFFFF);
      
   /* Interrupt Enable */
   CSP_ST16_SET_IER(st, intMask_u32);

   /* Configure ST controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}


/**
********************************************************************************
* @description	Enable ST Channel
*
* @return	None		
*
* @param    st				[in] Pointer to ST structure.
*			channel_u8		[in] Channel [0-7] to be enabled.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ST16Enable(CSP_ST16_T *const st, U8_T channel_u8)
{
   /* Enable ST Channel */
   CSP_ST16_SET_CR(st, (ST16_CHEN << channel_u8));
}
 


/**
*******************************************************************************
* @brief     Disable ST Channel
*
* @retval    None
*
* @param     st
* \n          input
* \n          brief description - Pointer to ST structure.
*
* @param     channel_u8
* \n          input
* \n          brief description - Channel [0-7] to be enabled.
*
* @pre       None
* @post      None
*
* @todo      Nothing
*
* @par InBodyDocs:
*******************************************************************************
*/
/**
********************************************************************************
* @description	Disable ST Channel
*
* @return	None		
*
* @param    st				[in] Pointer to ST structure.
*			channel_u8		[in] Channel [0-7] to be enabled.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ST16Disable(CSP_ST16_T *const st, U8_T channel_u8)
{
   /* Disable ST Channel */
   CSP_ST16_SET_CR(st, (ST16_CHDIS << channel_u8));
}

