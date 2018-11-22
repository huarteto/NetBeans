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
* @file name	csp_smc.c
* @description	Function declarations for SMC module
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
*		SMC/ /S3F4A0K/AGP1 EVB Rev1.0
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
* @description	Switch on the clock, reset registers and state machines,
*               and configure the SMC 
*
* @return	None		
*
* @param    SMC				[in] Pointer to SMC structure.
*			mode_u32		[in] Configure the SMC mode.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SMCInit(CSP_SMC_T *const SMC, U32_T mode_u32)
{
   /* Enable SMC Clock */  
   CSP_SMC_SET_ECR(SMC, SMC_SMC);
  
   /* SMC Software Reset */  
   CSP_SMC_SET_CR(SMC, SMC_SWRST);
  
   /* Configure SMC Mode */
   CSP_SMC_SET_MR(SMC, mode_u32);
}


/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None		
*
* @param    SMC				[in] Pointer to SMC structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SMCClose(CSP_SMC_T *const SMC)
{
   /* SMC Software Reset */  
   CSP_SMC_SET_CR(SMC, SMC_SWRST);

   /* Disable SMC Clock */  
   CSP_SMC_SET_DCR(SMC, SMC_SMC);
}


/**
********************************************************************************
* @description	Configure SMC Interrupts
*
* @return	None		
*
* @param    SMC				[in] Pointer to SMC structure.
*			intMode_u32		[in] Configure the priority level and source type.
*			intMask_u32		[in] Configure which interrupt bits are activated.
*			channelMask_u32	[in] Channel [0-31] to be configured.
*			callback_u32	[in] Function called through the assembler interrupt
*                                handler.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SMCConfigInterrupt(CSP_SMC_T *const SMC, 
                            U32_T intMode_u32, 
                            U32_T intMask_u32, 
                            U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum_u8 = 0;

   /* Get Peripheral Interrupt Number */
   intNum_u8 = CSP_GET_INT_NUMBER(SMC);

   /* Disable all interrupts */
   CSP_SMC_SET_IDR(SMC, 0xFFFFFFFF);

   /* Clear interrupts at SMC controller level */
   CSP_SMC_SET_CSR(SMC, SMC_CSR_MASK);

   /* Enable interrupt at SMC controller level */
   CSP_SMC_SET_IER(SMC, intMask_u32);

   /* Configure SMC controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}
                     

/**
********************************************************************************
* @description	Enable SMC
*
* @return	None		
*
* @param    SMC				[in] Pointer to SMC structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SMCEnable(CSP_SMC_T *const SMC)
{
   /* Enable SMC */
   CSP_SMC_SET_CR(SMC, SMC_SMCEN);
}  


/**
********************************************************************************
* @description	Disable SMC
*
* @return	None		
*
* @param    SMC				[in] Pointer to SMC structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_SMCDisable(CSP_SMC_T *const SMC)
{
   /* Disable SMC */
   CSP_SMC_SET_CR(SMC, SMC_SMCDIS);
}  
