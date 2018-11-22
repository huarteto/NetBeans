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
* @file name	csp_cinit.c
* @description	Copy RO code, RW data from ROM to RAM 
*				and zero-initialized the ZI data areas in RAM
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
*		S3F4A0K AGP1
*/ 

/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"

/*******************************************************************************
*************************  D E C L A R A T I O N S  ****************************
*******************************************************************************/
extern	U32_T Region$$Table$$Base;
extern	U32_T Region$$Table$$Limit;
extern	U32_T ZISection$$Table$$Base;
extern	U32_T ZISection$$Table$$Limit;

/*******************************************************************************
****************************  F U N C T I O N S  *******************************
*******************************************************************************/
/**
********************************************************************************
* @description	Copy RO code, RW data from ROM to RAM 
*				and zero-initialized the ZI data areas in RAM
*
* @return	None
*
* @param    None
*
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_CInit(void)
{
	/* Local variables */
   	U32_T index_u32;
   	U32_T source_u32;
   	U32_T destination_u32;
   	U32_T taille_u32;
   	U32_T i_u32;
   	U32_T start_u32;
   	U32_T end_u32;

	start_u32 = (U32_T) &Region$$Table$$Base;
   	end_u32   = (U32_T) &Region$$Table$$Limit;
   
   	/* For RO & RW sections */
   	for(index_u32 = start_u32; index_u32 < end_u32; index_u32 += (3*4))
   	{
    	source_u32      = *((U32_T*) (index_u32));
      	destination_u32 = *((U32_T*) (index_u32 + 4));
      	taille_u32      = *((U32_T*) (index_u32 + 8)) / 4;
      
      	for(i_u32=0; i_u32 < taille_u32; i_u32++)
      	{
        	*((U32_T*) (destination_u32)) =  *((U32_T*) (source_u32));
         	destination_u32 += 4;
         	source_u32      += 4;
      	} 
  	}
   
   	start_u32 = (U32_T) &ZISection$$Table$$Base;
   	end_u32   = (U32_T) &ZISection$$Table$$Limit;

	/* For ZI sections */
   	for(index_u32 = start_u32; index_u32 < end_u32; index_u32 += (2*4))
   	{
    	destination_u32 = *((U32_T*) (index_u32));
      	taille_u32      = *((U32_T*) (index_u32 + 4)) / 4;

		for(i_u32=0; i_u32 < taille_u32; i_u32++)
      	{
        	*((U32_T*) (destination_u32)) = 0;
         	destination_u32 += 4;
      	}
   	}    
}
