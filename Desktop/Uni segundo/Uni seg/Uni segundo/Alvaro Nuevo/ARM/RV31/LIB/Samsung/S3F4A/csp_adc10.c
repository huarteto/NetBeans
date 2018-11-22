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
* @file name	csp_adc10.c
* @description	Function declarations for ADC10 module
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
*		ADC/ /S3F4A0K/AGP1 EVB Rev1.0
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
* @description	Switch on the clock, reset the registers and configure 
*            	the ADC module mode and the LDMA RX line
*
* @return	None
*
* @param    adc		[in] Pointer to ADC structure.
*			mode_u32[in] Configure the ADC mode.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ADC10Init(CSP_ADC10_T *const adc, U32_T mode_u32)
{
	/* Enable ADC Clock */  
   	CSP_ADC10_SET_ECR(adc, ADC10_ADC);
  
 	/* ADC Software Reset */  
   	CSP_ADC10_SET_CR(adc, ADC10_SWRST);
  
   	/* Configure ADC Mode */
   	CSP_ADC10_SET_MR(adc, mode_u32);                       
}

/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None
*
* @param    adc		[in] Pointer to ADC structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ADC10Close(CSP_ADC10_T *const adc)
{
	/* ADC Software Reset */  
   	CSP_ADC10_SET_CR(adc, ADC10_SWRST);

   	/* Disable ADC Clock */  
   	CSP_ADC10_SET_DCR(adc, ADC10_ADC);
}

/**
********************************************************************************
* @description	Configure ADC Interrupts
*
* @return	None
*
* @param    adc			[in] Pointer to ADC structure.
*			Mode_u32	[in] Configure the priority level and source type.
*			Mask_u32	[in] Configure which interrupt bits are activated.
*			callback_u32[in] Function called through the assembler
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ADC10ConfigInterrupt(CSP_ADC10_T *const adc,U32_T intMode_u32, 
							       	U32_T intMask_u32, U32_T callback_u32)
{
	/* Local Variables */
   	U8_T intNum_u8 = 0;

	/* Get Peripheral Interrupt Number */
   	intNum_u8 = CSP_GET_INT_NUMBER(adc);

   	/* Disable all interrupt */
   	CSP_ADC10_SET_IDR(adc, 0xFFFFFFFF);
      
   	/* Interrupt Enable */
   	CSP_ADC10_SET_IER(adc, intMask_u32);

   	/* Configure ADC controller interrupt mode in GIC module */
   	CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
}

/**
********************************************************************************
* @description	Enable ADC
*
* @return	None
*
* @param    adc	[in] Pointer to ADC structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ADC10Enable(CSP_ADC10_T *const adc)
{
	/* Enable ADC */
   	CSP_ADC10_SET_CR(adc, ADC10_ADCEN);
}

/**
********************************************************************************
* @description	Disable ADC
*
* @return	None
*
* @param    adc	[in] Pointer to ADC structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ADC10Disable(CSP_ADC10_T *const adc)
{
	/* Disable ADC */  
   	CSP_ADC10_SET_CR(adc, ADC10_ADCDIS);
}

/**
********************************************************************************
* @description	Configure ADC module, start conversions on 10 bits ADC in 
*            	the chosen order, transfer results to memory thanks to PDC
*
* @return	None
*
* @param    adc			[in] Pointer to ADC structure.
*			data_p_u16	[in] Pointer to the address where the converted 
*                            values will be stored.
*			orderConversion0_u32 [in] Configure the order of conversion(CV1to8).
*			orderConversion1_u32 [in] Configure the order of conversion(CV9to16).
*			nbConversion_u8		 [in] Number of conversion to be carried out	
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ADC10StartConversion(CSP_ADC10_T *const adc, 
                              	U16_T *data_p_u16,
                              	U32_T orderConversion0_u32,
                              	U32_T orderConversion1_u32,  
                              	U8_T nbConversion_u8)
{
	/* Clear Number of Conversion */
   	CSP_ADC10_SET_MR(adc, (CSP_ADC10_GET_MR(adc) & ADC10_NBRCH_MASK));
  
   	/* Set New Input Configuration */
   	CSP_ADC10_SET_MR(adc,	(CSP_ADC10_GET_MR(adc) |
					((((U32_T)nbConversion_u8) << 16) & ADC10_NBRCH_MASK)));

	/* Clear Input Configuration */
   	CSP_ADC10_SET_CMR0(adc, 0);
   
   	/* Clear Input Configuration */
   	CSP_ADC10_SET_CMR1(adc, 0);  
      
   	/* Set New Input Configuration */
   	CSP_ADC10_SET_CMR0(adc, orderConversion0_u32);
   
   	/* Set New Input Configuration */
   	CSP_ADC10_SET_CMR1(adc, orderConversion1_u32);   
    
   	/* Start Conversion */
   	CSP_ADC10_SET_CR(adc, ADC10_START);

}

/**
********************************************************************************
* @description	Stop Current Conversion
*
* @return	None
*
* @param    adc			[in] Pointer to ADC structure.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_ADC10StopConversion(CSP_ADC10_T *const adc)
{
	/* Stop Conversion */
   	CSP_ADC10_SET_CR(adc, ADC10_STOP);
}


