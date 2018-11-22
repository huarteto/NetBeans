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
* @file name	cap_capt.c
* @description	Function declarations for Serial Peripheral Interface module
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
*		CAPT/ /S3F4A0K/AGP1 EVB Rev1.0
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
*           	CAPTURE module mode and the default corresponding LDMA RX line
*
* @return	None
*
* @param    capt		[in] Pointer to CAPT structure
*			mode_u32 	[in] Configure the CAPT mode
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CAPTInit(CSP_CAPT_T *const capt, U32_T mode_u32)
{
	/** Configures automatically the corresponding capture RX channel */
   	CSP_CAPTInitRxLdmaChannel(capt , mode_u32 ,
   										CSP_GET_LDMA_RX_NUMBER(capt));
}

/**
********************************************************************************
* @description	Switch on the clock, reset the registers and configure the  
*            	CAPTURE module mode and the LDMA RX line
*
* @return	None
*
* @param    capt		[in] Pointer to CAPT structure
*			mode_u32 	[in] Configure the CAPT mode
*			rxChannel_u8[in] Configure specific CAPTURE LDMA Channeluseful
* 							 when LDMA has to be synchronised on a different
*							 peripheral slower than the capture himself.
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CAPTInitRxLdmaChannel(CSP_CAPT_T *const capt, U32_T mode_u32,
									U8_T rxChannel_u8)
{
	/* Enable CAPTURE Clock */
   	CSP_CAPT_SET_ECR(capt, CAPT_CAP);
     
   	/* CAPTURE Software Reset */
   	CSP_CAPT_SET_CR(capt, CAPT_SWRST);

	/* Configure CAPTURE Mode */
   	CSP_CAPT_SET_MR(capt, mode_u32);
    
   	/* Configure CAPTURE LDMA Rx Line */
   	CSP_LDMARxInitChannel(LDMA0,
   							(U32_T)capt,
   							rxChannel_u8, 
                        	(LDMA_DEST | LDMA_DEST_INCR | LDMA_TRIG |
                         	LDMA_SIZE(LDMA_HALF_WORD_SIZE)));
}

/**
********************************************************************************
* @description	Reset and switch off the clock
*
* @return	None
*
* @param    capt	[in] Pointer to CAPT structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CAPTClose(CSP_CAPT_T *const capt)
{
	/* CAPTURE Software Reset */
   	CSP_CAPT_SET_CR(capt, CAPT_SWRST);

   	/* Disable CAPTURE Clock */  
   	CSP_CAPT_SET_DCR(capt, CAPT_CAP);
}

/**
********************************************************************************
* @description	Configure the CAPTURE Interrupts
*
* @return	None
*
* @param    capt		[in] Pointer to CAPT structure
*			intMode_u32 [in] Configure the priority level and source type
*			intMask_u32 [in] Configure which interrupt bits are activated.
*			callback_u32[in] Function called through the assembler
*							 interrupt handler.
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CAPTConfigInterrupt(CSP_CAPT_T *const capt, 
                            	U32_T intMode_u32, 
                             	U32_T intMask_u32, 
                             	U32_T callback_u32)
{
	/* Local Variables */
   	U8_T intNum_u8 = 0;

   	/* Get Peripheral Interrupt Number */
   	intNum_u8 = CSP_GET_INT_NUMBER(capt);

	/* Disable all interrupt */
   	CSP_CAPT_SET_IDR(capt, 0xFFFFFFFF);
      
   	/* Interrupt Enable */
   	CSP_CAPT_SET_IER(capt, intMask_u32);

   	/* Configure CAPTURE controller interrupt mode in GIC module */
   	CSP_GICConfigInterrupt(intNum_u8, intMode_u32, callback_u32);
   	
}

/**
********************************************************************************
* @description	Enable CAPT
*
* @return	None
*
* @param    capt	[in] Pointer to CAPT structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CAPTEnable(CSP_CAPT_T *const capt)
{
	/* Enable CAPTURE */
   	CSP_CAPT_SET_CR(capt, CAPT_CAPEN);
}

/**
********************************************************************************
* @description	Disable CAPT
*
* @return	None
*
* @param    capt	[in] Pointer to CAPT structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CAPTDisable(CSP_CAPT_T *const capt)
{
   /* Disable CAPTURE */
   CSP_CAPT_SET_CR(capt, CAPT_CAPDIS);
}

/**
********************************************************************************
* @description	Start CAPT
*
* @return	None
*
* @param    capt	[in] Pointer to CAPT structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CAPTStart(CSP_CAPT_T *const capt)
{
 	/* Start CAPTURE */
   	CSP_CAPT_SET_CR(capt, CAPT_STARTCAP);
} 

