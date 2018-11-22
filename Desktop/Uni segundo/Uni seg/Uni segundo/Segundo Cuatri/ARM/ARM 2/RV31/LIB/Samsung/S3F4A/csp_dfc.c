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
* @file name	csp_dfc.c
* @description	Function declarations for DFC module
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
*		DFC/ /S3F4A0K/AGP1 EVB Rev1.0
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
* @description	Configure DFC Interrupts
*
* @return	None
*
* @param    dfc			[in] Pointer to DFC structure
*			intMode_u32	[in] Configure the priority level and source type.
*			intMask_u32	[in] Configure which interrupt bits are activated 
*			callback_u32[in] Function called through the assembler 
*                            interrupt handler
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_DFC_ConfigInterrupt(CSP_DFC_T *const dfc, U32_T intMode_u32, 
                            	 U32_T intMask_u32, U32_T callback_u32)
{
	/* Local Variables */
   	U8_T intNum_u32 = 0;

   	/* Get Peripheral Interrupt Number */
   	intNum_u32= CSP_GET_INT_NUMBER(dfc);

   	/* Disable all interrupt */
   	CSP_DFC_SET_IDR(dfc, 0x00000007);
   	/* Interrupt Enable */
   	CSP_DFC_SET_IER(dfc,intMask_u32);
   
   	/* Configure DFC interrupt mode in GIC module */
   	CSP_GICConfigInterrupt(intMask_u32, intMode_u32, callback_u32);
}

/**
********************************************************************************
* @description	 Erase a specific page 
*
* @return	None
*
* @param    dfc			[in] Pointer to DFC structure
*			sector_u32	[in] Configure the sector number
*			page_u32	[in] Configure the page number 
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_DFC_PageErase(CSP_DFC_T *const dfc, U32_T sector_u32, 
													U32_T page_u32)
{
	CSP_DFC_SET_CR(dfc,	( DFC_SECTOR(sector_u32) 
							| DFC_PAGE(page_u32) 		
							| DFC_CRKEY 				 
                    		| DFC_ERASE(DFC_PAGE_ERASE)));
}

/**
********************************************************************************
* @description	 Erase a specific sector  
*
* @return	None
*
* @param    dfc			[in] Pointer to DFC structure
*			sector_u32	[in] Configure the sector number
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_DFC_SectorErase(CSP_DFC_T *const dfc, U32_T sector_u32)
{
	CSP_DFC_SET_CR(dfc, ( DFC_SECTOR(sector_u32)
							| DFC_CRKEY
							| DFC_ERASE(DFC_SECTOR_ERASE)));
}

/**
********************************************************************************
* @description	 Erase the entire Data Flash  
*
* @return	None
*
* @param    dfc	[in] Pointer to DFC structure
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_DFC_ChipErase(CSP_DFC_T *const dfc)
{
	CSP_DFC_SET_CR(dfc, (DFC_CRKEY | DFC_ERASE(DFC_CHIP_ERASE)));
}



