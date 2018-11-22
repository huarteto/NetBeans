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
* @file name	csp_wd.c
* @description	Function declarations for Watchdog module
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
*		WD/ /S3F4A0K/AGP1 EVB Rev1.0
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
* @description	Configure the WD module mode
*
* @return	None		
*
* @param    wd					[in] Pointer to WD structure
*			mode_u32			[in] Configure the mode of running and 
*									 especially the clock selection.
*			overflowMode_u32	[in] Configure the WatchDog Overflow Mode.
*			pendingMode_u32		[in] Configure the WatchDog Pending Window Mode.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_WDInit(CSP_WD_T *const wd, U32_T mode_u32, U32_T overflowMode_u32, 
                U32_T pendingMode_u32)
{
   /* Define Clock and WD Pending Window */
   CSP_WD_SET_MR(wd, (mode_u32));
   CSP_WD_SET_PWR(wd, (pendingMode_u32));

   /* Set Overflow Mode */
   CSP_WD_SET_OMR(wd, (overflowMode_u32));
}


/**
********************************************************************************
* @description	Configure WD Interrupts
*
* @return	None		
*
* @param    wd					[in] Pointer to WD structure
*			intMode_u32			[in] Configure the priority level and source type.
*			intMask_u32			[in] Configure which interrupt bits are activated.
*			callback_u32		[in] Function called through the assembler 
*                                    interrupt handler.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_WDConfigInterrupt(CSP_WD_T *const wd, 
                           U32_T intMode_u32, 
                           U32_T intMask_u32, 
                           U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum_u8 = 0;

   /* Get Peripheral Interrupt Number */
   intNum_u8 = CSP_GET_INT_NUMBER(wd);

   /* Disable all interrupt */
   CSP_WD_SET_IDR(wd, 0xFFFFFFFF);
   
   /* Interrupt always Enable */
   CSP_WD_SET_IER(wd, intMask_u32);

   /* Configure WD controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}


/**
********************************************************************************
* @description	Enable WatchDog
*
* @return	None		
*
* @param    wd				[in] Pointer to WD structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_WDEnable(CSP_WD_T *const wd)
{
   /* Enable WatchDog */
   CSP_WD_SET_OMR(wd, (CSP_WD_GET_OMR(wd) | WD_WDEN));
}


/**
********************************************************************************
* @description	Disable WatchDog
*
* @return	None		
*
* @param    wd				[in] Pointer to WD structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_WDDisable(CSP_WD_T *const wd)
{
   /* Disable WatchDog */
   CSP_WD_SET_OMR(wd, (CSP_WD_GET_OMR(wd) & (~WD_WDEN)));

}


/**
********************************************************************************
* @description	Restart Watchdog Counter
*
* @return	None		
*
* @param    wd				[in] Pointer to WD structure
*
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_WDCounterRestart(CSP_WD_T *const wd)
{
   /* Restart Key */
   CSP_WD_SET_CR(wd);
}
