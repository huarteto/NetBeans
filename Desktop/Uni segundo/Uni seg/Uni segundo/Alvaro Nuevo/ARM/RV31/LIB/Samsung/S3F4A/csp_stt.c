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
* @file name	csp_stt.c
* @description	Function declarations for Stamp Timer module
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
*		STT/ /S3F4A0K/AGP1 EVB Rev1.0
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
*               the STT module mode
*
* @return	None		
*
* @param    stt				[in] Pointer to STT structure.
*			mode_u32		[in] Configure when the counter should be reset.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_STTInit(CSP_STT_T *const stt, U32_T mode_u32)
{
   /* STT Software Reset */
   CSP_STT_SET_CR(stt, STT_SWRST);
  
   /* Configure the mode */
   CSP_STT_SET_MR(stt, mode_u32);
}


/**
********************************************************************************
* @description	Configure STT Interrupts
*
* @return	None		
*
* @param    stt				[in] Pointer to STT structure.
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
void CSP_STTConfigInterrupt(CSP_STT_T *const stt, 
                           U32_T intMode_u32, 
                           U32_T intMask_u32, 
                           U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum_u8 = 0;

   /* Get Peripheral Interrupt Number */
   intNum_u8 = CSP_GET_INT_NUMBER(stt);

   /* Disable all interrupt */
   CSP_STT_SET_IDR(stt, 0xFFFFFFFF);
   
   /* Interrupt always Enable */
   CSP_STT_SET_IER(stt, intMask_u32);

   /* Configure STT controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}


/**
********************************************************************************
* @description	Enable STT Seconds Counter and/or Alarm
*
* @return	None		
*
* @param    stt				[in] Pointer to STT structure.
*			enableMask_u8	[in] Configure which functions (Seconds Counter 
*                                and/or Alarm) are enabled.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_STTEnable(CSP_STT_T *const stt, U8_T enableMask_u8)
{
   /* Enable STT Seconds Counter and/or Alarm */
   CSP_STT_SET_CR(stt, enableMask_u8);
}


/**
********************************************************************************
* @description	Disable STT Seconds Counter and/or Alarm
*
* @return	None		
*
* @param    stt				[in] Pointer to STT structure.
*			disableMask_u8	[in] Configure which functions (Seconds Counter 
*                                and/or Alarm) are disabled.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_STTDisable(CSP_STT_T *const stt, U8_T disableMask_u8)
{
   /* Disable STT Seconds Counter and/or Alarm */
   CSP_STT_SET_CR(stt, disableMask_u8);
}


/**
********************************************************************************
* @description	Read the time passed since the last counter reset
*
* @return	32-bit value of the seconds counter	
*
* @param    stt				[in] Pointer to STT structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_STTGetTime(CSP_STT_T *const stt)
{
   /* Get time in seconds counter */
   return (CSP_STT_GET_CNTR(stt));
}


/**
********************************************************************************
* @description	Configure the seconds counter time
*
* @return	None
*
* @param    stt				[in] Pointer to STT structure.
*			time_u32		[in] Configure the time value.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_STTSetTime(CSP_STT_T *const stt, U32_T time_u32)
{
   /* Set time in seconds counter */
   CSP_STT_SET_CNTR(stt, time_u32);
}


/**
********************************************************************************
* @description	Read the Alarm time
*
* @return	32-bit value of the alarm time
*
* @param    stt				[in] Pointer to STT structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
U32_T CSP_STTGetAlarm(CSP_STT_T *const stt)
{
   /* Get Alarm time */
   return(CSP_STT_GET_ALR(stt));
}



/**
********************************************************************************
* @description	Configure the Alarm time. An interrupt can be generated 
*               when the seconds register reaches this value.
*
* @return	None
*
* @param    stt				[in] Pointer to STT structure.
*		    alarm_u32		[in] Configure the alarm time.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_STTSetAlarm(CSP_STT_T *const stt, U32_T alarm_u32)
{
   /* Set Alarm time */
   CSP_STT_SET_ALR(stt, alarm_u32);
}
