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
* @file name	csp_gic.c
* @description	Function declarations for GIC module
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
*		GIC/ /S3F4A0K/AGP1 EVB Rev1.0
*/ 

/*******************************************************************************
***************************  I N C L U D E S  **********************************
*******************************************************************************/
#include "csp.h"

extern __irq void CSP_GICSpuriousIntHandler (void);

/*******************************************************************************
**************************    V A R I A B L E S    *****************************
*******************************************************************************/
static U32_T spurious_counter_u32 = 0;

/*******************************************************************************
****************************  F U N C T I O N S  *******************************
*******************************************************************************/
/**
********************************************************************************
* @description	Initialize each GIC Source Vector and Source Mode to sensitive
*            	level and 0 priority
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
void CSP_GICInit(void)
{
	/* Local Variables */
   	U8_T i = 0;
  
  	/* Disable & CLear All Interrupts */
   	CSP_GIC_SET_IDCR(GIC, 0, 0xFFFFFFFF);
   	CSP_GIC_SET_IDCR(GIC, 1, 0xFFFFFFFF);
   	CSP_GIC_SET_ICCR(GIC, 0, 0xFFFFFFFF);
   	CSP_GIC_SET_ICCR(GIC, 1, 0xFFFFFFFF);

   	/* Reset source vector and source mode registers */
   	for(i = 0; i < NB_INTERRUPT; i++)
   	{
    	CSP_GIC_SET_SVR(GIC, i, 0x0);
      	CSP_GIC_SET_SMR(GIC, i, 0x0);
   	}
   
   	/* Initialize spurious interrupt vector handler */
   	CSP_GIC_SET_SPU(GIC, (U32_T) CSP_GICSpuriousIntHandler);
       
   	/* Validate all interrupts levels */
   	for(i = 0; i < NB_INTERRUPT_LEVEL; i++)
   	{
    	CSP_GIC_SET_EOICR(GIC, 0x1);
   	}
}

/**
********************************************************************************
* @description	Initialize each GIC Source Vector and Source Mode to sensitive
*            	level and 0 priority
*
* @return	None
*
* @param    intNum_u8	[in] Interrupt number [0-31] to be configured 
*			intMode_u32 [in] Configure the priority level and source type
*			callback_u32[in] Function called through the assembler 
*                            interrupt handler
* @pre		None
* @post	    None
*
* @todo     Nothing
*
********************************************************************************
*/
void CSP_GICConfigInterrupt(U8_T intNum_u8, 
							  U32_T intMode_u32, 
                              U32_T callback_u32)
{
	/* Local variable */
   	U8_T regNumber_u8 = 0;

	/* Configure interrupt mode in GIC module */
   	CSP_GIC_SET_SMR(GIC, intNum_u8, intMode_u32);
     
   	/* Configure interrupt vector in GIC module callback = interrupt handler */
   	CSP_GIC_SET_SVR(GIC, intNum_u8, callback_u32);

   	/* Enable Interrupt */
   	/* Determine the IECR register number where the interrupt is located */
   	regNumber_u8 = (U8_T)(intNum_u8 / 32);
   	/* Enable the corresponding interrupt */
   	CSP_GIC_SET_IECR(	GIC, regNumber_u8, 
    				  (0x1ul << (intNum_u8 - (regNumber_u8 * 32))));
}

/**
********************************************************************************
* @description	Undefined interrupt (this function should never be called).
*          		It just unstacks the interrupt
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
void CSP_GICUndefinedIntHandler(void)
{
	spurious_counter_u32++;
   	CSP_GIC_SET_EOICR(GIC, 0x1);
}
