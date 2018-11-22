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
* @file name	csp_i2c.c
* @description	Function declarations for I2C module
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
*		I2C/ /S3F4A0K/AGP1 EVB Rev1.0
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
* @description	Switch on the clock, reset the registers and configure the  
*            	I2C module mode.
*
* @return	None
*
* @param    i2c			[in] Pointer to I2C structure
*			mode_u32	[in] Configure the I2C mode
*			thold_u32	[in] Configure the Hold/Setup Delay
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_I2CInit(CSP_I2C_T *const i2c, U32_T mode_u32, U32_T thold_u32)
{
	/* Enable I2C Clock */
   	CSP_I2C_SET_ECR(i2c, I2C_I2C);

   	/* I2C Software Reset */
   	CSP_I2C_SET_CR(i2c, I2C_SWRST);

	/* Configure I2C Mode */
   	CSP_I2C_SET_MR(i2c, mode_u32);

  	/* Configure Hold/Setup Delay */
	CSP_I2C_SET_THOLD(i2c, thold_u32);
}

/**
********************************************************************************
* @description	Switch on the clock, reset the registers and configure the  
*           	I2C module mode.
*
* @return	None
*
* @param    i2c			[in] Pointer to I2C structure
*			
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_I2CClose(CSP_I2C_T *const i2c)
{
	/* I2C Software Reset */
   	CSP_I2C_SET_CR(i2c, I2C_SWRST);

   	/* Disable I2C Clock */
   	CSP_I2C_SET_DCR(i2c, I2C_I2C);
}
/**
********************************************************************************
* @description	Configure I2C Interrupts
*
* @return	None
*
* @param    i2c			[in] Pointer to I2C structure
*			intMode_u32	[in] Configure the priority level and source type
*			intMask_u32	[in] Configure which interrupt bits are activated
*			callback_u32[in] Function called through the assembler
*                          interrupt handler.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_I2CConfigInterrupt(CSP_I2C_T *const i2c, 
                           		U32_T intMode_u32, 
                            	U32_T intMask_u32, 
                            	U32_T callback_u32)
{
	/* Local Variables */
   	U8_T intNum_u8 = 0;

   	/* Get Peripheral Interrupt Number */
   	intNum_u8 = CSP_GET_INT_NUMBER(i2c);

   	/* Disable all interrupt */
   	CSP_I2C_SET_IDR(i2c, 0xFFFFFFFF);
   
   	/* Interrupt Enable */
   	CSP_I2C_SET_IER(i2c, intMask_u32);

	/* Configure I2C controller interrupt mode in GIC module */
   CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}


/**
********************************************************************************
* @description	Configure I2C in Master or Slave mode and specify Slave Response 
*            	address with General Call detection
*
* @return	None
*
* @param    i2c			[in] Pointer to I2C structure
*			cr_u16		[in] Configure the I2C Slave or Master mode
*			add_u8		[in] Configure address for Slave Response
*                            and general call detection
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_I2CModeConfig(CSP_I2C_T *const i2c, U16_T cr_u16, U8_T add_u8)
{
	/* Configure I2C Control Register */
   	CSP_I2C_SET_CR(i2c, cr_u16);
	
   	/* Configure ADR Field and GC bit */
   	CSP_I2C_SET_ADR(i2c, add_u8);
      
}

