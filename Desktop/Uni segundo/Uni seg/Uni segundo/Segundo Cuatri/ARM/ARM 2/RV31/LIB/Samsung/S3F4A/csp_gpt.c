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
* @file name	csp_gpt.c
* @description	Function declarations for General Purpose Timer module
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
*		GPT/ /S3F4A0K/AGP1 EVB Rev1.0
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
* @description	Switch on the clock, reset the registers and
*				configure the GPT module mode
*
* @return	None
*
* @param    gpt				[in] Pointer to GPT structure
*			mode_u32		[in] Configure the GPT Mode
*			regValue_p_u16	[in] Pointer on array where counter values of 
*                                RA, RB and RC will be defined
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_GPTInit(CSP_GPT_T *const gpt,
					U32_T mode_u32,
					U16_T *regValue_p_u16)
{
	/* Enable GPT Clock */
   	CSP_GPT_SET_ECR(gpt, GPT_GPT);

	/* GPT Software Reset */
   	CSP_GPT_SET_CR(gpt, GPT_SWRST);
   	/* Configure GPT Mode */
   	CSP_GPT_SET_MR(gpt, mode_u32);

   	/* Set Registers Value */
   	CSP_GPT_SET_RA(gpt, regValue_p_u16[0]);
   	CSP_GPT_SET_RB(gpt, regValue_p_u16[1]);
   	CSP_GPT_SET_RC(gpt, regValue_p_u16[2]);
}

/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None
*
* @param    gpt				[in] Pointer to GPT structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_GPTClose(CSP_GPT_T *const gpt)
{
	/* GPT Software Reset */
   	CSP_GPT_SET_CR(gpt, GPT_SWRST);

   	/* Disable GPT Clock */  	
   	CSP_GPT_SET_DCR(gpt, GPT_GPT);
}

/**
********************************************************************************
* @description	Configure GPT Interrupts
*
* @return	None
*
* @param    gpt				[in] Pointer to GPT structure
*			intMode_u32		[in] Configure the priority level and source type
*			intMask_u32		[in] Configure which interrupt bits are activated
*			callback_u32	[in] Function called through the assembler 
*                                interrupt handler
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_GPTConfigInterrupt(CSP_GPT_T *const gpt, 
                           		U32_T intMode_u32, 
                            	U32_T intMask_u32, 
                            	U32_T callback_u32)
{
	/* Local Variables */
   	U8_T intChannel_u8 = 0;
   	U8_T intNumber_u8 = 0;

   	/* Get Peripheral Interrupt Number */
   	intChannel_u8= (U8_T)(((U32_T)gpt & 0xFFFul) >> 8u);
   	intNumber_u8 = CSP_GET_INT_NUMBER_CHANNEL(gpt, intChannel_u8);

   	/* Disable all interrupt */
   	CSP_GPT_SET_IDR(gpt, 0xFFFFFFFF);
   
   	/* Interrupt Enable */
   	CSP_GPT_SET_IER(gpt, intMask_u32);

   	/* Configure GPT controller interrupt mode in GIC module */
   	CSP_GICConfigInterrupt(intNumber_u8, intMode_u32, callback_u32);
}

/**
********************************************************************************
* @description	Enable GPT
*
* @return	None
*
* @param    gpt				[in] Pointer to GPT structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/

void CSP_GPTEnable(CSP_GPT_T *const gpt)
{
	/* Enable GPT */ 
   	CSP_GPT_SET_CR(gpt, GPT_CLKEN | GPT_SWTRG);
}

/**
********************************************************************************
* @description	Disable GPT
*
* @return	None
*
* @param    gpt				[in] Pointer to GPT structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_GPTDisable(CSP_GPT_T *const gpt)
{
	/* Disable GPT */
   	CSP_GPT_SET_CR(gpt, GPT_CLKDIS);
}








