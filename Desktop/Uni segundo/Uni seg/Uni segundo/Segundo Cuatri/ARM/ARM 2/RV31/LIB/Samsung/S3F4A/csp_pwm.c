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
* @file name	csp_pwm.c
* @description	Function declarations for Pulse Width Modulation module
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
*		PWM/ /S3F4A0K/AGP1 EVB Rev1.0
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
* @description	Switch on the clock, reset the registers
*
* @return	None		
*
* @param    pwm				[in] Pointer to PWM structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PWMInit(CSP_PWM_T *const pwm)
{
   /* Enable PWM Clock */
   CSP_PWM_SET_ECR(pwm, PWM_PWM);
 
   /* PWM Software Reset */
   CSP_PWM_SET_CR(pwm, PWM_SWRST);

}

/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None		
*
* @param    pwm				[in] Pointer to PWM structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PWMClose(CSP_PWM_T *const pwm)
{
   /* PWM Software Reset */
   CSP_PWM_SET_CR(pwm, PWM_SWRST);

   /* Disable PWM Clock */
   CSP_PWM_SET_DCR(pwm, PWM_PWM);
}


/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None		
*
* @param    pwm				[in] Pointer to PWM structure.
*			channel_u8		[in] Channel [0-3] to be configured.
*			mode_u32		[in] Configure the PWM mode (Prescalar + PL).
*			delay_u16		[in] Configure the PWM delay. Number of cycles 
*                                during which the output is inactive.
*			pulse_u16		[in] Configure the PWM pulse. Number of cycles 
*                                during which the output is active.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PWMConfigureChannel(CSP_PWM_T *const pwm, 
                               U8_T channel_u8, 
                               U32_T mode_u32, 
                               U16_T delay_u16,
                               U16_T pulse_u16)
{
   /* Configure channel mode */
   CSP_PWM_SET_MR(pwm, (CSP_PWM_GET_MR(pwm) & 
                         ~(0xFFul << (8u * channel_u8))));
   CSP_PWM_SET_MR(pwm, (CSP_PWM_GET_MR(pwm) | 
                         (mode_u32 << (8u * channel_u8))));
      
   /* Configure PWM Signal */
   CSP_PWM_CHANNEL_SET_DLY(pwm, channel_u8, delay_u16);
   CSP_PWM_CHANNEL_SET_PUL(pwm, channel_u8, pulse_u16);
}


/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None		
*
* @param    pwm				[in] Pointer to PWM structure.
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
void CSP_PWMConfigInterrupt(CSP_PWM_T *const pwm, 
                              U32_T intMode_u32, 
                              U32_T intMask_u32, 
                              U32_T callback_u32)
{
   /* Local Variables */
   U8_T intNum_u8 = 0;

   /* Get Peripheral Interrupt Number */
   intNum_u8 = CSP_GET_INT_NUMBER(pwm);

   /* Disable all interrupt */
   CSP_PWM_SET_IDR(pwm, 0xFFFFFFFF);
      
   /* Interrupt Enable */
   CSP_PWM_SET_IER(pwm, intMask_u32);

   /* Configure PWM controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}


/**
********************************************************************************
* @description	Enable PWM Channel
*
* @return	None		
*
* @param    pwm				[in] Pointer to PWM structure.
*			channel_u8   	[in] Channel [0-3] to be enabled.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PWMEnable(CSP_PWM_T *const pwm , U8_T channel_u8)
{
   /* Enable PWM Channel */
   CSP_PWM_SET_CR(pwm, (PWM_PWMEN << (8u * channel_u8)));
}


/**
********************************************************************************
* @description	Disable PWM Channel
*
* @return	None		
*
* @param    pwm				[in] Pointer to PWM structure.
*			channel_u8   	[in] Channel [0-3] to be enabled.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PWMDisable(CSP_PWM_T *const pwm, U8_T channel_u8)
{
   /* Disable PWM Channel */
   CSP_PWM_SET_CR(pwm, (PWM_PWMDIS << (8u * channel_u8)));
}


/**
********************************************************************************
* @description	Stop and reset PWM channel specified, leaving output pin in 
*            	the offstate state
*
* @return	None		
*
* @param    pwm				[in] Pointer to PWM structure.
*			channel_u8   	[in] Channel [0-3] to be enabled.
*			offstate_u8   	[in] Set LOW (0) or HIGH (1) level on output pin 
*                                when the PWM channel is stopped.
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_PWMSetOffstate(CSP_PWM_T *const pwm, 
                        U8_T channel_u8, 
                        U8_T offstate_u8)
{
   /* Init PWM Module */
   if(offstate_u8 == 0)
   {
      /* Offstate Level = 0 */
      CSP_PWMConfigureChannel(pwm, channel_u8, 0x10, 0x1, 0x0);
   }
   else
   {
      /* Offstate Level = 1 */
      CSP_PWMConfigureChannel(pwm, channel_u8, 0x10, 0x0, 0x1);
   }   

   /* Start PWM Module */
   CSP_PWM_SET_CR(pwm, (PWM_PWMEN << (8u * channel_u8)));
}




